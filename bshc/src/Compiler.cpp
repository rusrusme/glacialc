//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Compiler.hpp"

#include <memory>

#include "Log.hpp"
#include "Utility.hpp"
#include "platform/dx12/DX12Compiler.hpp"
#include "platform/opengl/GLCompiler.hpp"
#include "platform/vulkan/VulkanCompiler.hpp"

namespace bshc {
    Unique<Compiler> Compiler::create(const RenderBackend& render_backend) {
        switch (render_backend) {
            case RenderBackend::OpenGL :
                return std::make_unique<GLCompiler>();
            case RenderBackend::Vulkan :
                return std::make_unique<VulkanCompiler>();
            case RenderBackend::DX12 :
                return std::make_unique<DX12Compiler>();
        }
        BSH_ERROR("Invalid render backend: {0}", to_string(render_backend));
        return nullptr;
    }
}
