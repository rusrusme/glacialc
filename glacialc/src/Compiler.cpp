//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "glacialc/Compiler.hpp"

#include <memory>

#include "glacialc/Log.hpp"
#include "glacialc/platform/dx12/DX12Compiler.hpp"
#include "glacialc/platform/opengl/GLCompiler.hpp"
#include "glacialc/platform/vulkan/VulkanCompiler.hpp"

namespace glacialc {
    Unique<Compiler> Compiler::create(const RenderBackend& render_backend) {
        switch (render_backend) {
            case RenderBackend::OpenGL :
                return std::make_unique<GLCompiler>();
            case RenderBackend::Vulkan :
                return std::make_unique<VulkanCompiler>();
            case RenderBackend::DX12 :
                return std::make_unique<DX12Compiler>();
        }
        GSH_ERROR("Invalid render backend: {0}", to_string(render_backend));
        return nullptr;
    }
}
