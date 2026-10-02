//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>

namespace bshc {
    constexpr uint32_t BSH_SIGNATURE = 0x42485342;

    enum class ShaderStage : uint8_t {
        VERTEX   = 0,
        FRAGMENT = 1,
        GEOMETRY = 2,
        COMPUTE  = 3
    };

    #pragma pack(push, 1)
    struct BSHHeader {
        uint32_t signature = BSH_SIGNATURE;
        uint16_t version = 1;
        uint8_t stage_count = 0;

        // Abstracted Render States
        uint8_t depth_test  : 1 = 1;
        uint8_t depth_write : 1 = 1;
        uint8_t cull_mode   : 2 = 2; // 0 = None, 1 = Front, 2 = Back
        uint8_t blend_mode  : 2 = 0; // 0 = Opaque, 1 = Alpha, etc.
        uint8_t reserved    : 2 = 0;
    };

    struct BSHStageHeader {
        ShaderStage stage_type;
        uint8_t is_spirv; // 1 = SPIR-V Bytecode, 0 = Raw GLSL Text
        uint32_t data_size;
    };
    #pragma pack(pop)
} // bshc
