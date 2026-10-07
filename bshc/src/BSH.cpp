//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <filesystem>
#include <fstream>

#include "Compiler.hpp"
#include "Log.hpp"
#include "Preprocessor.hpp"

using namespace bshc;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        BSH_MESSAGE("Usage: bshc <shader.bsh> [-o <output_path.bshb>]");
        return 0;
    }

    std::filesystem::path input_path = argv[1];
    std::filesystem::path output_path = input_path;
    output_path.replace_extension(".bshb");

    for (int i = 2; i < argc; ++i) {
        std::string_view arg = argv[i];
        if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            output_path = argv[++i];
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

    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    Preprocessor preprocessor(input_path.parent_path());
    ParsedData parsed_data = preprocessor.process(source, input_path);

    if (!parsed_data.success) {
        BSH_ERROR("Preprocessing Error: {}", parsed_data.error_message);
        return 1;
    }

    Compiler compiler;
    if (!compiler.compile_to_file(parsed_data, output_path, input_path.filename().string())) {
        BSH_ERROR("Failed to compile and write binary to {}", output_path.string());
        return 1;
    }

    BSH_MESSAGE("Successfully compiled {} -> {}", input_path.filename().string(), output_path.filename().string());
    return 0;
}
