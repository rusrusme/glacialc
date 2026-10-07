//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bshc {
    struct CompilerOutput {
        bool success = true;
        std::string error_message;
        std::vector<uint8_t> binary_data;
    };
} // bshc