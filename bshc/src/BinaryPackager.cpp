//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "BinaryPackager.hpp"

#include <fstream>

namespace bshc {
    std::vector<uint8_t> BinaryPackager::package_binary(BSHHeader header, const std::vector<CompiledStage>& stages) {
        header.stage_count = static_cast<uint8_t>(stages.size());
        std::vector<uint8_t> buffer;
        const size_t header_section_size = sizeof(BSHHeader) + sizeof(BSHStageHeader) * stages.size();
        size_t total_payload_size = 0;

        for (const auto& [stage, payload] : stages) {
            total_payload_size += payload.size();
        }
        buffer.reserve(header_section_size + total_payload_size);
        const auto* header_bytes = reinterpret_cast<const uint8_t*>(&header);
        buffer.insert(buffer.end(), header_bytes, header_bytes + sizeof(BSHHeader));

        auto current_offset = static_cast<uint32_t>(header_section_size);
        for (const auto& [stage, payload] : stages) {
            BSHStageHeader stage_header{};
            stage_header.stage_type = stage;
            stage_header.data_offset = current_offset;
            stage_header.data_size = static_cast<uint32_t>(payload.size());

            const auto* stage_bytes = reinterpret_cast<const uint8_t*>(&stage_header);
            buffer.insert(buffer.end(), stage_bytes, stage_bytes + sizeof(BSHStageHeader));
            current_offset += stage_header.data_size;
        }
        for (const auto& [stage, payload] : stages) {
            buffer.insert(buffer.end(), payload.begin(), payload.end());
        }
        return buffer;
    }

    bool BinaryPackager::save(const std::filesystem::path& path, const std::vector<uint8_t>& binary_data) {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) return false;

        file.write(reinterpret_cast<const char*>(binary_data.data()), static_cast<std::streamsize>(binary_data.size()));
        return file.good();
    }
} // bshc
