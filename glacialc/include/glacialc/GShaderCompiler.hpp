//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

#include "glacialc/Compiler.hpp"

namespace glacialc {
    struct BatchJob {
        std::filesystem::path input;
        std::filesystem::path output;
    };

    class GShaderCompiler {
    public:
        static std::vector<uint8_t> compile_source(std::string_view source,
                                             RenderBackend target_backend,
                                             const std::filesystem::path& include_dir = "");

        static bool compile_file(const std::filesystem::path& input_path,
                                 const std::filesystem::path& output_path,
                                 RenderBackend target_backend);

        static bool compile_batch(const std::vector<BatchJob>& jobs,
                                  RenderBackend target_backend);
    };
}
