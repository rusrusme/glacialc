//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "platform/opengl/GLCompiler.hpp"

#include <regex>

namespace bshc {
    CompilerOutput GLCompiler::compile(const ParsedData& data) {
        CompilerOutput output;
        output.success = true;

        if (data.stages.empty()) {
            output.success = false;
            output.error_message = "GLCompiler: No shader stages found in ParsedData.";
            return output;
        }

        output.stages.reserve(data.stages.size());

        for (const auto& [stage, source] : data.stages) {
            std::string glsl_code = process_glsl_source(source, stage);
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
        glsl.reserve(raw_source.size() + 64);

        if (!raw_source.starts_with("#version")) {
            glsl += "#version 450 core\n\n";
        }

        std::string source_str{raw_source};

        static const std::regex set_qualifier_regex(R"(set\s*=\s*\d+\s*,?\s*)");
        source_str = std::regex_replace(source_str, set_qualifier_regex, "");

        static const std::regex empty_comma_regex(R"(layout\s*\(\s*,\s*)");
        source_str = std::regex_replace(source_str, empty_comma_regex, "layout(");

        glsl += source_str;
        return glsl;
    }
} // bshc
