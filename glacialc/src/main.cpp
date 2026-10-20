//
// Copyright (c) 2026 rusrusme
// All rights reserved.
//

#include <filesystem>
#include <string_view>
#include <vector>

#include "glacialc/GShader.hpp"
#include "glacialc/GShaderCompiler.hpp"
#include "glacialc/Log.hpp"
#include "glacialc/Utility.hpp"

using namespace glacialc;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        GSH_MESSAGE("Usage: glacialc <shader.gshader> [shader.gshader ...] [-o <dir> | <file>] [-b <opengl|vulkan|dx12>]");
        return 0;
    }

    std::vector<std::filesystem::path> inputs;
    std::filesystem::path explicit_out;
    auto target_backend = RenderBackend::OpenGL;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if ((arg == "-b" || arg == "--backend") && i + 1 < argc) {
            std::string_view backend_str = argv[++i];
            auto parsed_backend = parse_backend_string(backend_str);
            if (!parsed_backend) {
                GSH_ERROR("Invalid backend target: '{}'. Valid options: opengl, vulkan, dx12", backend_str);
                return 1;
            }
            target_backend = *parsed_backend;
        }
        else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            explicit_out = argv[++i];
        }
        else if (!arg.starts_with('-')) {
            inputs.emplace_back(arg);
        }
    }

    if (inputs.empty()) {
        GSH_ERROR("No input shader files specified.");
        return 1;
    }

    std::vector<BatchJob> jobs;
    jobs.reserve(inputs.size());

    const bool is_multi_file = inputs.size() > 1;

    for (const auto& input : inputs) {
        std::filesystem::path out_path;

        if (!explicit_out.empty()) {
            if (is_multi_file || std::filesystem::is_directory(explicit_out) || explicit_out.extension().empty()) {
                out_path = input.is_relative() ? explicit_out / input : explicit_out / input.filename();
                out_path.replace_extension(".gsb");
            }
            else {
                out_path = explicit_out;
            }
        }
        else {
            out_path = input;
            out_path.replace_extension(".gsb");
        }

        jobs.push_back({.input = input, .output = out_path});
    }

    const bool success = GShaderCompiler::compile_batch(jobs, target_backend);
    return success ? 0 : 1;
}
