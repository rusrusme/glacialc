//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "Preprocessor.hpp"

namespace bshc {
    struct CompileResult {
        bool success = true;
        std::string error_message;
        std::vector<uint8_t> binary_data;
    };

    class Compiler {
    public:
        Compiler() = default;

        CompileResult compile(const ParseResult& parse_result, std::string_view source_filename = "shader.bsh");
        bool compile_to_file(const ParseResult& parse_result, const std::filesystem::path& output_path, std::string_view source_filename = "shader.bsh");
    };
} // bshc
