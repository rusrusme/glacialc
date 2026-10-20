//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "glacialc/platform/dx12/DX12Compiler.hpp"

#include <spirv_hlsl.hpp>
#include <shaderc/shaderc.hpp>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <unknwn.h>
#include <dxcapi.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#endif

namespace glacialc {
    static const wchar_t* get_dxc_target_profile(const ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex : return L"vs_6_0";
            case ShaderStage::Fragment : return L"ps_6_0";
            case ShaderStage::Geometry : return L"gs_6_0";
            case ShaderStage::Compute : return L"cs_6_0";
        }
        return L"vs_6_0";
    }

    CompilerOutput DX12Compiler::compile(const ParsedData& data) {
        CompilerOutput output;
        output.success = true;

        if (data.stages.empty()) {
            output.success = false;
            output.error_message = "DX12Compiler: No shader stages found in ParsedData.";
            return output;
        }

        const shaderc::Compiler shaderc_compiler;
        shaderc::CompileOptions shaderc_options;
        shaderc_options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

        shaderc_options.AddMacroDefinition("GSHADER_DX12", "1");
        shaderc_options.AddMacroDefinition("GSHADER_BACKEND_DX12", "1");
        shaderc_options.AddMacroDefinition("gl_BaseInstanceARB", "gl_BaseInstance");

        #if defined(_WIN32)
        ComPtr<IDxcUtils> dxc_utils;
        ComPtr<IDxcCompiler3> dxc_compiler;

        if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils))) ||
            FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler)))) {
            output.success = false;
            output.error_message = "DX12Compiler: Failed to initialize DirectX Shader Compiler (DXC).";
            return output;
        }
        #endif

        output.stages.reserve(data.stages.size());

        for (const auto& [stage, source] : data.stages) {
            std::string glsl;
            if (!source.starts_with("#version")) {
                constexpr std::string_view header = "#version 450 core\n\n";
                glsl.reserve(header.size() + source.size());
                glsl += header;
                glsl += source;
            }
            else {
                glsl = source;
            }

            shaderc::SpvCompilationResult spirv_result = shaderc_compiler.CompileGlslToSpv(
                glsl, to_shaderc_stage(stage), "shader.gshader", shaderc_options
            );

            if (spirv_result.GetCompilationStatus() != shaderc_compilation_status_success) {
                output.success = false;
                output.error_message = "DX12 Stage 1 (GLSL->SPIR-V) Error:\n";
                output.error_message += spirv_result.GetErrorMessage();
                return output;
            }

            std::vector spirv_words(spirv_result.cbegin(), spirv_result.cend());
            spirv_cross::CompilerHLSL hlsl_compiler(std::move(spirv_words));

            spirv_cross::CompilerHLSL::Options hlsl_options;
            hlsl_options.shader_model = 60;
            hlsl_compiler.set_hlsl_options(hlsl_options);

            std::string hlsl_source;
            try {
                hlsl_source = hlsl_compiler.compile();
            }
            catch (const std::exception& e) {
                output.success = false;
                output.error_message = "DX12 Stage 2 (SPIR-V->HLSL) Error:\n";
                output.error_message += e.what();
                return output;
            }

            std::vector<uint8_t> payload;

            #if defined(_WIN32)
            DxcBuffer source_buffer{
                .Ptr      = hlsl_source.data(),
                .Size     = hlsl_source.size(),
                .Encoding = DXC_CP_UTF8
            };

            const wchar_t* target_profile = get_dxc_target_profile(stage);
            std::vector dxc_args = {
                L"shader.hlsl",
                L"-E", L"main",
                L"-T", target_profile,
                L"-O3",
                DXC_ARG_WARNINGS_ARE_ERRORS
            };

            ComPtr<IDxcResult> dxc_result;
            HRESULT hr = dxc_compiler->Compile(
                &source_buffer,
                dxc_args.data(),
                static_cast<uint32_t>(dxc_args.size()),
                nullptr,
                IID_PPV_ARGS(&dxc_result)
            );

            HRESULT status_hr = S_OK;
            if (dxc_result) {
                dxc_result->GetStatus(&status_hr);
            }

            ComPtr<IDxcBlobUtf8> errors;
            if (dxc_result) {
                dxc_result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
            }

            if (FAILED(hr) || FAILED(status_hr)) {
                output.success = false;
                output.error_message = "DX12 Stage 3 (HLSL->DXIL) Error:\n";
                if (errors && errors->GetStringLength() > 0) {
                    output.error_message += errors->GetStringPointer();
                }
                else {
                    output.error_message += "Unknown DXC compilation failure.";
                }
                return output;
            }

            ComPtr<IDxcBlob> dxil_blob;
            if (FAILED(dxc_result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&dxil_blob), nullptr)) || !dxil_blob) {
                output.success = false;
                output.error_message = "DX12 Stage 3 (HLSL->DXIL) Error: Failed to retrieve DXIL byte blob.";
                return output;
            }

            const auto* dxil_begin = static_cast<const uint8_t*>(dxil_blob->GetBufferPointer());
            const auto* dxil_end = dxil_begin + dxil_blob->GetBufferSize();
            payload.assign(dxil_begin, dxil_end);
            #else
            payload.assign(hlsl_source.begin(), hlsl_source.end());
            #endif

            output.stages.push_back(CompiledStage{
                .stage   = stage,
                .payload = std::move(payload)
            });
        }
        return output;
    }
} // glacialc
