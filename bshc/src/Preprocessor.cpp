//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Preprocessor.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace bshc {
    static std::string_view trim(std::string_view sv) {
        while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.front()))) {
            sv.remove_prefix(1);
        }
        while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.back()))) {
            sv.remove_suffix(1);
        }
        return sv;
    }

    Preprocessor::Preprocessor(std::filesystem::path include_dir) : include_dir_(std::move(include_dir)) {}

    ParsedData Preprocessor::process(std::string_view source, const std::filesystem::path& current_file_path) {
        ParsedData result;
        included_files_.clear();
        active_includes_.clear();

        if (!current_file_path.empty()) {
            const std::string root_canonical = std::filesystem::canonical(current_file_path).string();
            active_includes_.insert(root_canonical);
        }

        std::string global_buffer;
        std::vector<std::pair<ShaderStage, std::string>> stage_buffers;
        int active_stage_idx = -1;

        process_source(source, current_file_path, result.header, global_buffer, stage_buffers, active_stage_idx, result);

        if (!result.success) {
            return result;
        }

        result.header.stage_count = static_cast<uint8_t>(stage_buffers.size());

        for (auto& [stage, stage_code] : stage_buffers) {
            std::string final_source = global_buffer;
            final_source += '\n';
            final_source += stage_code;
            result.stages.push_back(CompiledStageSource{
                .stage       = stage,
                .source = std::move(final_source)
            });
        }

        return result;
    }

    void Preprocessor::process_source(std::string_view source, const std::filesystem::path& current_file_path, BSHHeader& header, std::string& global_buffer, std::vector<std::pair<ShaderStage, std::string>>& stage_buffers, int& active_stage_idx, ParsedData& result) {
        std::stringstream ss{std::string(source)};
        std::string line;

        while (std::getline(ss, line)) {
            std::string_view trimmed = trim(line);

            if (trimmed.empty()) {
                if (active_stage_idx == -1) {
                    global_buffer += '\n';
                }
                else {
                    stage_buffers[active_stage_idx].second += '\n';
                }
                continue;
            }

            // Pragmas (#pragma once)
            if (trimmed == "#pragma once") {
                if (!current_file_path.empty()) {
                    included_files_.insert(std::filesystem::canonical(current_file_path).string());
                }
                continue;
            }

            // Stage Directives (#vert, #frag, #geom, #comp)
            if (trimmed == "#vert" || trimmed == "#frag" || trimmed == "#geom" || trimmed == "#comp") {
                auto stage = ShaderStage::Vertex;
                if (trimmed == "#frag") stage = ShaderStage::Fragment;
                else if (trimmed == "#geom") stage = ShaderStage::Geometry;
                else if (trimmed == "#comp") stage = ShaderStage::Compute;

                stage_buffers.emplace_back(stage, "");
                active_stage_idx = static_cast<int>(stage_buffers.size()) - 1;
                continue;
            }

            // Include Directives (#include "...")
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

                // Circular Dependency Guard
                if (active_includes_.contains(canonical_path)) {
                    result.success = false;
                    result.error_message = "Circular #include dependency detected: " + canonical_path;
                    return;
                }

                // Skip if guarded by #pragma once
                if (included_files_.contains(canonical_path)) {
                    continue;
                }

                std::ifstream file(target_path);
                if (!file.is_open()) {
                    result.success = false;
                    result.error_message = "Failed to open include file: " + target_path.string();
                    return;
                }

                std::string included_content((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());

                // Push onto active include stack for recursion, then pop
                active_includes_.insert(canonical_path);
                process_source(included_content, target_path, header, global_buffer, stage_buffers, active_stage_idx, result);
                active_includes_.erase(canonical_path);

                if (!result.success) {
                    return;
                }
                continue;
            }

            // 5. Regular GLSL Code Line
            if (active_stage_idx == -1) {
                global_buffer += line + "\n";
            }
            else {
                stage_buffers[active_stage_idx].second += line + "\n";
            }
        }
    }
} // bshc
