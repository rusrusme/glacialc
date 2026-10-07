//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Preprocessor.hpp"
#include "Utility.hpp"

namespace bshc {
    struct CompiledStage {
        ShaderStage stage;
        std::vector<uint8_t> payload;
    };

    struct CompilerOutput {
        bool success = true;
        std::string error_message;
        std::vector<CompiledStage> stages;
    };

    class Compiler {
    public:
        Compiler() = default;
        virtual ~Compiler() = default;

        virtual CompilerOutput compile(const ParsedData& data) = 0;

        static Unique<Compiler> create(const RenderBackend& render_backend);
    };
}
