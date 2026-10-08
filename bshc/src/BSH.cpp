//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <algorithm>
#include <atomic>
#include <execution>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

#include "BinaryPackager.hpp"
#include "Compiler.hpp"
#include "Log.hpp"
#include "Preprocessor.hpp"
#include "Utility.hpp"

using namespace bshc;

namespace {
    struct ShaderJob {
        std::filesystem::path input;
        std::filesystem::path output;
    };
}

static bool compile_single_shader(const std::filesystem::path& input_path,
                                  const std::filesystem::path& output_path,
                                  RenderBackend target_backend) {
    if (!std::filesystem::exists(input_path)) {
        BSH_ERROR("Input file does not exist: {}", input_path.string());
        return false;
    }

    std::ifstream file(input_path);
    if (!file.is_open()) {
        BSH_ERROR("Failed to open file: {}", input_path.string());
        return false;
    }

    std::string source((std::istreambuf_iterator(file)), {});

    Preprocessor preprocessor(input_path.parent_path());
    ParsedData parsed = preprocessor.process(source, input_path);

    if (!parsed.success) {
        BSH_ERROR("Preprocessing Error [{}]: {}", input_path.filename().string(), parsed.error_message);
        return false;
    }

    parsed.header.backend = target_backend;

    Unique<Compiler> compiler = Compiler::create(target_backend);
    if (!compiler) {
        BSH_ERROR("Unsupported or uncompiled render backend: {}", to_string(target_backend));
        return false;
    }

    CompilerOutput compiled = compiler->compile(parsed);
    if (!compiled.success) {
        BSH_ERROR("Compilation Error [{}]: {}", input_path.filename().string(), compiled.error_message);
        return false;
    }

    if (output_path.has_parent_path()) {
        std::filesystem::create_directories(output_path.parent_path());
    }

    auto bin = BinaryPackager::package_binary(parsed.header, compiled.stages);
    if (!BinaryPackager::save(output_path, bin)) {
        BSH_ERROR("Failed to write binary output file: {}", output_path.string());
        return false;
    }

    BSH_MESSAGE("Successfully compiled {} [{}] to {}",
                input_path.filename().string(),
                to_string(target_backend),
                output_path.filename().string());
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        BSH_MESSAGE("Usage: bshc <shader1.bsh> [shader2.bsh ...] [-o <dir> | <file>] [-b <opengl|vulkan|dx12>]");
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
                BSH_ERROR("Invalid backend target: '{}'. Valid options: opengl, vulkan, dx12", backend_str);
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
        BSH_ERROR("No input shader files specified.");
        return 1;
    }

    std::vector<ShaderJob> jobs;
    jobs.reserve(inputs.size());

    const bool is_multi_file = inputs.size() > 1;

    for (const auto& input : inputs) {
        std::filesystem::path out_path;

        if (!explicit_out.empty()) {
            if (is_multi_file || std::filesystem::is_directory(explicit_out) || explicit_out.extension().empty()) {
                out_path = explicit_out / input.filename();
                out_path.replace_extension(".bshb");
            }
            else {
                out_path = explicit_out;
            }
        }
        else {
            out_path = input;
            out_path.replace_extension(".bshb");
        }

        jobs.push_back({.input = input, .output = out_path});
    }

    std::atomic has_error{false};

    std::for_each(std::execution::par, jobs.begin(), jobs.end(), [&](const ShaderJob& job) {
        if (!compile_single_shader(job.input, job.output, target_backend)) {
            has_error = true;
        }
    });
    return has_error ? 1 : 0;
}
