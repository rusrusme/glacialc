//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <string>
#include <vector>

#include "Preprocessor.hpp"
#include "glacialc/Utility.hpp"

namespace glacialc {
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
