//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <execution>
#include <filesystem>
#include <fstream>

#include "BinaryPackager.hpp"
#include "Compiler.hpp"
#include "Log.hpp"
#include "Preprocessor.hpp"
#include "Utility.hpp"

using namespace bshc;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        BSH_MESSAGE("Usage: bshc <shader.bsh> [-o <output_path.bshb>] [-b <opengl|vulkan|dx12>]");
        return 0;
    }

    std::filesystem::path input_path = argv[1];
    std::filesystem::path output_path = input_path;
    output_path.replace_extension(".bshb");

    auto target_backend = RenderBackend::OpenGL;

    for (int i = 2; i < argc; ++i) {
        std::string_view arg = argv[i];
        if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            output_path = argv[++i];
        }
        else if ((arg == "-b" || arg == "--backend") && i + 1 < argc) {
            std::string_view backend_str = argv[++i];
            auto parsed_backend = parse_backend_string(backend_str);
            if (!parsed_backend) {
                BSH_ERROR("Invalid backend target: '{}'. Valid options: opengl, vulkan, dx12", backend_str);
                return 1;
            }
            target_backend = *parsed_backend;
        }
    }

    if (!std::filesystem::exists(input_path)) {
        BSH_ERROR("Input file does not exist: {}", input_path.string());
        return 1;
    }

    std::ifstream file(input_path);
    if (!file.is_open()) {
        BSH_ERROR("Failed to open file: {}", input_path.string());
        return 1;
    }

    std::string source((std::istreambuf_iterator(file)), {});

    Preprocessor preprocessor(input_path.parent_path());
    ParsedData parsed = preprocessor.process(source, input_path);

    if (!parsed.success) {
        BSH_ERROR("Preprocessing Error: {}", parsed.error_message);
        return 1;
    }

    parsed.header.backend = target_backend;

    Unique<Compiler> compiler = Compiler::create(target_backend);
    if (!compiler) {
        BSH_ERROR("Unsupported or uncompiled render backend: {}", to_string(target_backend));
        return 1;
    }

    CompilerOutput compiled = compiler->compile(parsed);
    if (!compiled.success) {
        BSH_ERROR("Compilation Error: {}", compiled.error_message);
        return 1;
    }

    auto bin = BinaryPackager::package_binary(parsed.header, compiled.stages);
    if (!BinaryPackager::save(output_path, bin)) {
        BSH_ERROR("Failed to write binary output file: {}", output_path.string());
        return 1;
    }

    BSH_MESSAGE("Successfully compiled {} [{}] to {}",
                input_path.filename().string(),
                to_string(target_backend),
                output_path.filename().string());
    return 0;
}
