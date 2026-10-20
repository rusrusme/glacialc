//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>

namespace glacialc {
    constexpr uint32_t GSH_SIGNATURE = 0x43485347;

    enum class RenderBackend : uint8_t {
        OpenGL = 0,
        Vulkan = 1,
        DX12   = 2
    };

    enum class ShaderStage : uint8_t {
        Vertex   = 0,
        Fragment = 1,
        Geometry = 2,
        Compute  = 3
    };

    #pragma pack(push, 1)
    struct GShaderHeader {
        uint32_t signature = GSH_SIGNATURE;
        uint16_t version = 1;
        RenderBackend backend = RenderBackend::OpenGL;
        uint8_t stage_count = 0;

        uint8_t unassigned[8] = {}; // not assigned
    };

    struct GShaderStageHeader {
        uint32_t data_offset = 0;
        uint32_t data_size = 0;
        ShaderStage stage_type = ShaderStage::Vertex;

        uint8_t unassigned[7] = {}; // not assigned
    };
    #pragma pack(pop)

    // sanity check
    static_assert(sizeof(GShaderHeader) == 16, "GShaderHeader size must be 16 bytes");
    static_assert(sizeof(GShaderStageHeader) == 16, "GShaderStageHeader size must be 16 bytes");
} // glacialc
