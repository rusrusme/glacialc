//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "bshc/Compiler.hpp"

namespace bshc {
    class BinaryPackager {
    public:
        static std::vector<uint8_t> package_binary(BSHHeader header, const std::vector<CompiledStage>& stages);
        static bool save(const std::filesystem::path& path, const std::vector<uint8_t>& binary_data);
    };
} // bshc
