//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "BSH.hpp"

namespace bshc {
    struct CompiledStageSource {
        ShaderStage stage;
        std::string source;
    };

    struct ParsedData {
        BSHHeader header;
        std::vector<CompiledStageSource> stages;
        bool success = true;
        std::string error_message;
    };

    class Preprocessor {
    public:
        explicit Preprocessor(std::filesystem::path include_dir = ".");
        ~Preprocessor() = default;

        ParsedData process(std::string_view source, const std::filesystem::path& current_file_path = "");

    private:
        void process_source(std::string_view source,
                            const std::filesystem::path& current_file_path,
                            BSHHeader& header,
                            std::string& global_buffer,
                            std::vector<std::pair<ShaderStage, std::string>>& stage_buffers,
                            int& active_stage_idx,
                            ParsedData& result);

        std::filesystem::path include_dir_;
        std::unordered_set<std::string> included_files_;  // Files guarded by #pragma once
        std::unordered_set<std::string> active_includes_; // Current recursion stack for cycle detection
    };
} // bshc
