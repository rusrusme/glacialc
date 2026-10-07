//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "Compiler.hpp"

namespace bshc {
    class GLCompiler : public Compiler {
    public:
        GLCompiler() = default;
        ~GLCompiler() override = default;

        CompilerOutput compile(const ParsedData& data) override;
    };
} // bshc
