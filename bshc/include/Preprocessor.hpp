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
    struct PreprocessedStage {
        ShaderStage stage;
        std::string source;
    };

    struct ParsedData {
        BSHHeader header;
        std::vector<PreprocessedStage> stages;
        bool success = true;
        std::string error_message;
    };

    class Preprocessor {
    public:
        explicit Preprocessor(std::filesystem::path include_dir = ".");
        ~Preprocessor() = default;

        ParsedData process(const std::string_view& source, const std::filesystem::path& current_file_path = "");

    private:
        void process_source(std::string_view source,
                            const std::filesystem::path& current_file_path,
                            std::string& global_buffer,
                            std::vector<std::pair<ShaderStage, std::string>>& stage_buffers,
                            int active_stage_idx,
                            ParsedData& result,
                            std::unordered_set<std::string>& included_files,
                            std::unordered_set<std::string>& active_includes);

        std::filesystem::path include_dir_;
    };
} // bshc
