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
    };

    struct BSHStageHeader {
        ShaderStage stage_type;
        uint32_t data_size;
    };
    #pragma pack(pop)
} // bshc
