//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "glacialc/platform/opengl/GLCompiler.hpp"

#include <shaderc/shaderc.hpp>

namespace glacialc {
    CompilerOutput GLCompiler::compile(const ParsedData& data) {
        CompilerOutput output;
        output.success = true;

        if (data.stages.empty()) {
            output.success = false;
            output.error_message = "GLCompiler: No shader stages found in ParsedData.";
            return output;
        }

        const shaderc::Compiler validator;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);

        options.AddMacroDefinition("GSHADER_OPENGL", "1");
        options.AddMacroDefinition("GSHADER_BACKEND_OPENGL", "1");

        output.stages.reserve(data.stages.size());

        for (const auto& [stage, source] : data.stages) {
            std::string glsl_code = process_glsl_source(source, stage);

            auto result = validator.CompileGlslToSpv(glsl_code, to_shaderc_stage(stage), "shader.gshader", options);
            if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
                output.success = false;
                output.error_message = "GLSL Syntax Error:\n" + result.GetErrorMessage();
                return output;
            }

            std::vector<uint8_t> payload(glsl_code.begin(), glsl_code.end());
            output.stages.push_back(CompiledStage{
                .stage   = stage,
                .payload = std::move(payload)
            });
        }

        return output;
    }

    std::string GLCompiler::process_glsl_source(std::string_view raw_source, ShaderStage stage) {
        std::string glsl;
        glsl.reserve(raw_source.size() + 128);

        constexpr std::string_view ext_header = "#extension GL_ARB_shader_draw_parameters : enable\n";

        if (!raw_source.starts_with("#version")) {
            glsl += "#version 450 core\n";
            glsl += ext_header;
            glsl += '\n';
            glsl += raw_source;
        }
        else {
            const size_t newline = raw_source.find('\n');
            if (newline != std::string_view::npos) {
                glsl.append(raw_source.substr(0, newline + 1));
                glsl += ext_header;
                glsl.append(raw_source.substr(newline + 1));
            }
            else {
                glsl += raw_source;
                glsl += '\n';
                glsl += ext_header;
            }
        }
        return glsl;
    }
} // glacialc
