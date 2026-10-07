//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "platform/vulkan/vulkanCompiler.hpp"

#include <shaderc/shaderc.hpp>

#include "BSH.hpp"

namespace bshc {
    static shaderc_shader_kind to_shaderc_stage(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::VERTEX : return shaderc_glsl_vertex_shader;
            case ShaderStage::FRAGMENT : return shaderc_glsl_fragment_shader;
            case ShaderStage::GEOMETRY : return shaderc_glsl_geometry_shader;
            case ShaderStage::COMPUTE : return shaderc_glsl_compute_shader;
        }
        return shaderc_glsl_infer_from_source;
    }

} // bshc