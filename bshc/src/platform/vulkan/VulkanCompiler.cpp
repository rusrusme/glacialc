//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "platform/vulkan/VulkanCompiler.hpp"

#include <shaderc/shaderc.hpp>

namespace bshc {
    CompilerOutput VulkanCompiler::compile(const ParsedData& data) {
        CompilerOutput output;
        output.success = true;

        if (data.stages.empty()) {
            output.success = false;
            output.error_message = "VulkanCompiler: No shader stages found in ParsedData.";
            return output;
        }

        const shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

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

            shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(
                glsl,
                to_shaderc_stage(stage),
                "shader.bsh",
                options
            );

            if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
                output.success = false;

                output.error_message = "Vulkan SPIR-V Error in stage ";
                output.error_message += std::to_string(static_cast<int>(stage));
                output.error_message += ":\n";
                output.error_message += result.GetErrorMessage();
                return output;
            }

            const auto* spirv_begin = reinterpret_cast<const uint8_t*>(result.cbegin());
            const auto* spirv_end = reinterpret_cast<const uint8_t*>(result.cend());

            std::vector payload(spirv_begin, spirv_end);

            output.stages.push_back(CompiledStage{
                .stage   = stage,
                .payload = std::move(payload)
            });
        }
        return output;
    }
} // bshc
