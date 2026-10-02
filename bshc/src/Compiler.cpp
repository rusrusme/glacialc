//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Compiler.hpp"

#include <fstream>
#include <shaderc/shaderc.hpp>

namespace bshc {
    static shaderc_shader_kind to_shaderc_stage(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::VERTEX : return shaderc_glsl_vertex_shader;
            case ShaderStage::FRAGMENT : return shaderc_glsl_fragment_shader;
            case ShaderStage::GEOMETRY : return shaderc_glsl_geometry_shader;
            case ShaderStage::COMPUTE : return shaderc_glsl_compute_shader;
        }
        return shaderc_glsl_infer_from_source;
    }

    CompileResult Compiler::compile(const ParseResult& parse_result, std::string_view source_filename) {
        CompileResult result;

        if (!parse_result.success) {
            result.success = false;
            result.error_message = "Cannot compile invalid preprocessor result: " + parse_result.error_message;
            return result;
        }

        const shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        // Vulkan 1.3 Target
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

        std::vector<uint8_t>& buffer = result.binary_data;

        // 1. Write File Header (BSHHeader)
        const auto* header_bytes = reinterpret_cast<const uint8_t*>(&parse_result.header);
        buffer.insert(buffer.end(), header_bytes, header_bytes + sizeof(BSHHeader));

        // 2. Compile each stage and append StageHeader + SPIR-V Blob
        for (const auto& [stage, glsl_source] : parse_result.stages) {
            const shaderc_shader_kind kind = to_shaderc_stage(stage);

            shaderc::SpvCompilationResult spv_result = compiler.CompileGlslToSpv(
                glsl_source,
                kind,
                std::string(source_filename).c_str(),
                options
            );

            if (spv_result.GetCompilationStatus() != shaderc_compilation_status_success) {
                result.success = false;
                result.error_message = spv_result.GetErrorMessage();
                result.binary_data.clear();
                return result;
            }

            // SPIR-V words (uint32_t) converted to raw bytes
            const uint32_t* spv_begin = spv_result.cbegin();
            const uint32_t* spv_end = spv_result.cend();
            const auto* spv_bytes = reinterpret_cast<const uint8_t*>(spv_begin);
            const size_t spv_byte_size = (spv_end - spv_begin) * sizeof(uint32_t);

            // Populate Stage Header
            BSHStageHeader stage_header{
                .stage_type = stage,
                .is_spirv   = 1,
                .data_size  = static_cast<uint32_t>(spv_byte_size)
            };

            const auto* stage_header_bytes = reinterpret_cast<const uint8_t*>(&stage_header);
            buffer.insert(buffer.end(), stage_header_bytes, stage_header_bytes + sizeof(BSHStageHeader));

            // Append SPIR-V Payload
            buffer.insert(buffer.end(), spv_bytes, spv_bytes + spv_byte_size);
        }

        return result;
    }

    bool Compiler::compile_to_file(const ParseResult& parse_result, const std::filesystem::path& output_path, const std::string_view source_filename) {
        const CompileResult result = compile(parse_result, source_filename);
        if (!result.success) return false;

        std::ofstream out_file(output_path, std::ios::binary);
        if (!out_file.is_open()) return false;

        out_file.write(reinterpret_cast<const char*>(result.binary_data.data()), result.binary_data.size());
        return true;
    }
} // bshc
