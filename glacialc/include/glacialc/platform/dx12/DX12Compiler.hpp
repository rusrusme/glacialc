//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include "glacialc/Compiler.hpp"

namespace glacialc {
    class DX12Compiler : public Compiler {
    public:
        DX12Compiler() = default;
        ~DX12Compiler() override = default;

        CompilerOutput compile(const ParsedData& data) override;
    };
} // glacialc
