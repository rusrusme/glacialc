//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>

#include "BSH.hpp"

namespace bshc {
    template<typename T>
    using Shared = std::shared_ptr<T>;

    template<typename T>
    using Unique = std::unique_ptr<T>;

    constexpr const char* to_string(const RenderBackend& backend) {
        switch (backend) {
            case RenderBackend::OpenGL : return "OpenGL";
            case RenderBackend::Vulkan : return "Vulkan";
            case RenderBackend::DX12 : return "DX12";
            default : return "Unknown";
        }
    }

    static std::optional<RenderBackend> parse_backend_string(const std::string_view str) {
        if (str == "opengl" || str == "gl") return RenderBackend::OpenGL;
        if (str == "vulkan" || str == "vk") return RenderBackend::Vulkan;
        if (str == "dx12"   || str == "d3d12" || str == "directx12") return RenderBackend::DX12;
        return std::nullopt;
    }
}
