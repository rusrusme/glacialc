//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <memory>
#include <optional>
#include <shaderc/shaderc.h>

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

    static std::string_view trim(std::string_view sv) {
        while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.front()))) {
            sv.remove_prefix(1);
        }
        while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.back()))) {
            sv.remove_suffix(1);
        }
        return sv;
    }

    static shaderc_shader_kind to_shaderc_stage(const ShaderStage stage) {
        switch (stage) {
            case ShaderStage::Vertex   : return shaderc_glsl_vertex_shader;
            case ShaderStage::Fragment : return shaderc_glsl_fragment_shader;
            case ShaderStage::Geometry : return shaderc_glsl_geometry_shader;
            case ShaderStage::Compute  : return shaderc_glsl_compute_shader;
        }
        return shaderc_glsl_infer_from_source;
    }
}
