//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Preprocessor.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "Utility.hpp"

namespace bshc {
    Preprocessor::Preprocessor(std::filesystem::path include_dir)
        : include_dir_(std::move(include_dir)) {}

    ParsedData Preprocessor::process(const std::string_view& source, const std::filesystem::path& current_file_path) {
        ParsedData result;
        std::unordered_set<std::string> included_files;
        std::unordered_set<std::string> active_includes;

        if (!current_file_path.empty() && std::filesystem::exists(current_file_path)) {
            active_includes.insert(std::filesystem::canonical(current_file_path).string());
        }

        std::string global_buffer;
        std::vector<std::pair<ShaderStage, std::string>> stage_buffers;

        process_source(source, current_file_path, global_buffer, stage_buffers, -1, result, included_files, active_includes);

        if (!result.success) { return result; }

        result.header.stage_count = static_cast<uint8_t>(stage_buffers.size());

        result.stages.reserve(stage_buffers.size());
        for (auto& [stage, stage_code] : stage_buffers) {
            std::string final_source = global_buffer;
            if (!global_buffer.empty() && !global_buffer.ends_with('\n')) {
                final_source += '\n';
            }
            final_source += stage_code;

            result.stages.push_back(PreprocessedStage{
                .stage  = stage,
                .source = std::move(final_source)
            });
        }
        return result;
    }

    void Preprocessor::process_source(
        std::string_view source,
        const std::filesystem::path& current_file_path,
        std::string& global_buffer,
        std::vector<std::pair<ShaderStage, std::string>>& stage_buffers,
        int active_stage_idx,
        ParsedData& result,
        std::unordered_set<std::string>& included_files,
        std::unordered_set<std::string>& active_includes) {
        // -----------------------------------------------
        std::stringstream ss{std::string(source)};
        std::string line;
        bool in_block_comment = false;

        while (std::getline(ss, line)) {
            std::string_view trimmed = trim(line);

            if (in_block_comment) {
                size_t comment_end = trimmed.find("*/");
                if (comment_end != std::string_view::npos) {
                    in_block_comment = false;
                    trimmed = trim(trimmed.substr(comment_end + 2));
                }
                else {
                    if (active_stage_idx == -1) global_buffer += '\n';
                    else stage_buffers[active_stage_idx].second += '\n';
                    continue;
                }
            }

            size_t comment_start = trimmed.find("/*");
            if (comment_start != std::string_view::npos) {
                size_t comment_end = trimmed.find("*/", comment_start + 2);
                if (comment_end != std::string_view::npos) {
                    std::string line_without_comment = std::string(trimmed.substr(0, comment_start)) +
                        std::string(trimmed.substr(comment_end + 2));
                    trimmed = trim(line_without_comment);
                }
                else {
                    in_block_comment = true;
                    trimmed = trim(trimmed.substr(0, comment_start));
                }
            }

            if (trimmed.starts_with("//") || trimmed.empty()) {
                if (active_stage_idx == -1) global_buffer += '\n';
                else stage_buffers[active_stage_idx].second += '\n';
                continue;
            }

            if (trimmed == "#pragma once") {
                if (!current_file_path.empty() && std::filesystem::exists(current_file_path)) {
                    included_files.insert(std::filesystem::canonical(current_file_path).string());
                }
                continue;
            }

            if (trimmed == "#vert" || trimmed == "#frag" || trimmed == "#geom" || trimmed == "#comp") {
                auto stage = ShaderStage::Vertex;
                if (trimmed == "#frag") stage = ShaderStage::Fragment;
                else if (trimmed == "#geom") stage = ShaderStage::Geometry;
                else if (trimmed == "#comp") stage = ShaderStage::Compute;

                stage_buffers.emplace_back(stage, "");
                active_stage_idx = static_cast<int>(stage_buffers.size()) - 1;
                continue;
            }

            if (trimmed.starts_with("#include")) {
                size_t first_quote = trimmed.find('"');
                size_t last_quote = trimmed.rfind('"');

                if (first_quote == std::string_view::npos || last_quote == first_quote) {
                    result.success = false;
                    result.error_message = "Malformed #include directive: " + std::string(line);
                    return;
                }

                std::string rel_path_str{trimmed.substr(first_quote + 1, last_quote - first_quote - 1)};
                std::filesystem::path target_path = include_dir_ / rel_path_str;

                if (!current_file_path.empty() && current_file_path.has_parent_path()) {
                    std::filesystem::path local_target = current_file_path.parent_path() / rel_path_str;
                    if (std::filesystem::exists(local_target)) {
                        target_path = local_target;
                    }
                }

                if (!std::filesystem::exists(target_path)) {
                    result.success = false;
                    result.error_message = "Could not find included file: " + target_path.string();
                    return;
                }

                std::string canonical_path = std::filesystem::canonical(target_path).string();

                if (active_includes.contains(canonical_path)) {
                    result.success = false;
                    result.error_message = "Circular #include dependency detected: " + canonical_path;
                    return;
                }

                if (included_files.contains(canonical_path)) {
                    continue;
                }

                std::string included_content;
                {
                    std::ifstream file(target_path);
                    if (!file.is_open()) {
                        result.success = false;
                        result.error_message = "Failed to open include file: " + target_path.string();
                        return;
                    }
                    included_content = std::string(std::istreambuf_iterator(file), {});
                }

                active_includes.insert(canonical_path);
                process_source(included_content, target_path, global_buffer, stage_buffers, active_stage_idx, result, included_files, active_includes);
                active_includes.erase(canonical_path);

                if (!result.success) {
                    return;
                }
                continue;
            }

            if (active_stage_idx == -1) {
                global_buffer += line + "\n";
            }
            else {
                stage_buffers[active_stage_idx].second += line + "\n";
            }
        }
    }
} // bshc
