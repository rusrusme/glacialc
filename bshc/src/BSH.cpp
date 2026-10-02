//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <filesystem>
#include <fstream>
#include <iostream>

#include "Compiler.hpp"
#include "Preprocessor.hpp"

using namespace bshc;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Basis Shader Compiler (bshc)\n";
        std::cout << "Usage: bshc <input_path.bsh> [-o <output_path.bshb>]\n";
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
        std::cerr << "Error: Input file does not exist: " << input_path.string() << "\n";
        return 1;
    }

    std::ifstream file(input_path);
    if (!file.is_open()) {
        std::cerr << "Error: Failed to open file: " << input_path.string() << "\n";
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    Preprocessor preprocessor(input_path.parent_path());
    ParseResult parse_result = preprocessor.process(source, input_path);

    if (!parse_result.success) {
        std::cerr << "Preprocessing Error:\n" << parse_result.error_message << "\n";
        return 1;
    }

    Compiler compiler;
    if (!compiler.compile_to_file(parse_result, output_path, input_path.filename().string())) {
        std::cerr << "Error: Failed to compile and write binary to " << output_path.string() << "\n";
        return 1;
    }

    std::cout << "Successfully compiled " << input_path.filename().string()
        << " -> " << output_path.filename().string() << "\n";

    return 0;
}
