# glacialc — Glacial Shader Compiler

**glacialc** is a lightweight, high-performance, platform-agnostic shader compilation pipeline written in C++20. It allows game developers to write unified multi-stage shaders (`gshader`) and compile them into a compact, contiguous binary format (`gsb`) targeting **OpenGL**, **Vulkan**, and **DirectX 12**

---

## Key Features

* **Write-Once, Target-Anywhere:** Write standard C/GLSL-style shaders without platform-specific decorations
* **Unified Pipeline:** Single-file source supporting multi-stage shaders (`#vert`, `#frag`, `#geom`, `#comp`)
* **Robust Preprocessor:**
    * Recursive `#include` resolution with file-relative path priorities
    * File-guard tracking (`#pragma once`)
    * Circular dependency detection and prevention
    * Comment-aware directive parsing (`//` and `/* ... */`)
* **Multi-Backend Transpilation:**
    * **OpenGL:** Validated GLSL (`#version 450 core`)
    * **Vulkan:** High-performance **SPIR-V** binary via Google `shaderc`
    * **DirectX 12:** Transpiled **DXIL** bytecode via `shaderc` -> `SPIRV-Cross` -> HLSL 60 -> Microsoft `DXC`
* **Zero-Copy Contiguous Binary (`bshb`):** Packed 16-byte aligned headers with offset tables for instant runtime loading

---

## Glacial Shader Language Syntax (`.gshader`)

GSH shaders use standard GLSL syntax with a flat resource binding model. Code written outside stage directives is automatically treated as **global scope** and prepended to all stages

```glsl
// =========================================================
// Global Scope (Shared across all stages)
// =========================================================
layout(binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

layout(binding = 1) uniform sampler2D u_MainTexture;

// =========================================================
// Vertex Stage
// =========================================================
#vert
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 0) out vec2 v_TexCoord;

void main() {
    v_TexCoord = a_TexCoord;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

// =========================================================
// Fragment Stage
// =========================================================
#frag
layout(location = 0) in vec2 v_TexCoord;
layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = texture(u_MainTexture, v_TexCoord);
}
```

---

## Building glacialc
### Requirements
- C++20 Compiler (MSVC 19.30+, Clang 13+, GCC 11+)
- CMake 3.20+
- Vulkan SDK (Provides shaderc)
- SPIRV-Cross
- DirectX Shader Compiler (DXC) (Windows SDK)
---
CMake Build Instructions
```bash
git clone https://github.com/rusrusme/glacialc.git
cd GlacialShaderCompiler
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Adding glacialc to a Project
```cmake
include(FetchContent)
FetchContent_Declare(
        glacialc
        GIT_REPOSITORY https://github.com/rusrusme/glacialc.git
        GIT_TAG v1.0.2
)
FetchContent_MakeAvailable(glacialc)
# ...
target_link_libraries(<PROJECT> PRIVATE glacialc::glacialc_core)
```

---
CLI Command Line Usage
```bash
# General Usage
bshc <input.gshader> [-o <output.gsb>] [-b <backend>]

# Compile for OpenGL
bshc shaders/basic.gshader -o bin/basic_gl.gsb -b opengl

# Compile for Vulkan
bshc shaders/basic.gshader -o bin/basic_vk.gsb -b vulkan

# Compile for DirectX 12
bshc shaders/basic.gshader -o bin/basic_dx12.gsb -b dx12
```
---
CLI Arguments
- <input.gshader> : Path to input GSH shader source file

- -o, --output : (Optional) Path to output binary file

- -b, --backend : Target backend: opengl / gl, vulkan / vk, dx12 / d3d12 Defaults to opengl
