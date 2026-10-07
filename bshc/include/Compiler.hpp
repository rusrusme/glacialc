//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#if defined(RENDER_API_OPENGL)
#include "platform/opengl/GLCompiler.hpp"

namespace bshc {
    using Compiler = GLCompiler;
}
#elif defined(RENDER_API_vulkan)
#include "platform/vulkan/vulkanCompiler.hpp"

namespace bshc {
    using Compiler = VulkanCompiler;
}
#elif defined(RENDER_API_DX12)
#include "platform/dx12/DX12Compiler.hpp"

namespace bshc {
    using Compiler = DX12Compiler;
}
#endif
