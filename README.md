# glacialc

glacialc is a C++20 shader compiler that packs multi-stage shaders into a single binary (.gsb) targeting OpenGL, Vulkan,
and DirectX 12.

Instead of maintaining separate shader files for each stage and backend, you write standard GLSL in one .gshader file.
glacialc handles preprocessing, stage splitting, and transpilation down to driver-ready bytecode.

---

## Features

- **Single-file workflows:** Define vertex, fragment, geometry, and compute stages (#vert, #frag, #geom, #comp) in one
  file with shared global scope.
- **Preprocessor:** Recursive #include resolution relative to the source file, #pragma once support, circular dependency
  detection, and comment stripping.
- **Multiple backends:**
    - OpenGL: Validated GLSL (#version 450 core)
    - Vulkan: SPIR-V binary via shaderc
    - DirectX 12: DXIL bytecode via shaderc to SPIRV-Cross to HLSL 6.0 and lastly DXC
- **Fast runtime loading:** Exports to a 16-byte aligned, packed binary format (.gsb) designed for zero-copy file-to-GPU
  memory mapping.

---

## Shader Format (`.gshader`)

Code outside explicit stage directives is placed in the global scope and shared across all stages:

```glsl
#pragma once
// Shared across all stages
#common
layout(binding = 0) uniform Camera {
    mat4 u_ViewProjection;
};

layout(binding = 1) uniform sampler2D u_MainTexture;

#vert
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 0) out vec2 v_TexCoord;

void main() {
    v_TexCoord = a_TexCoord;
    gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#frag
layout(location = 0) in vec2 v_TexCoord;
layout(location = 0) out vec4 FragColor;

void main() {
    FragColor = texture(u_MainTexture, v_TexCoord);
}
```

---

## Building

### Requirements

- C++20 Compiler (MSVC 19.30+, Clang 13+, GCC 11+)
- CMake 3.20+
- Vulkan SDK (Provides shaderc)
- SPIRV-Cross
- DirectX Shader Compiler (dxc, included with the Windows SDK)

```bash
git clone https://github.com/rusrusme/glacialc.git
cd glacialc
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Integration (CMake)

```cmake
include(FetchContent)
FetchContent_Declare(
        glacialc
        GIT_REPOSITORY https://github.com/rusrusme/glacialc.git
        GIT_TAG v1.4.0
)
FetchContent_MakeAvailable(glacialc)

target_link_libraries(${PROJECT_NAME} PRIVATE glacialc::glacialc_core)
```

## CLI Usage

```bash
glacialc <input.gshader> [options]
```

### Options

| Flag            | Description                                                               | Default     |
|-----------------|---------------------------------------------------------------------------|-------------|
| 'input.gshader' | Path to input GSH shader source file                                      | required    |
| -o, --output    | (Optional) Path to output binary file                                     | 'input.gsb' |
| -b, --backend   | Target backend: opengl / gl, vulkan / vk, dx12 / d3d12 Defaults to opengl | opengl      |

### Examples

#### OpenGL:

```bash
glacialc shaders/basic.gshader -o bin/basic_gl.gsb -b opengl
```

#### Vulkan:

```bash
glacialc shaders/basic.gshader -o bin/basic_vk.gsb -b vulkan
```

#### DirectX 12:

```bash
glacialc shaders/basic.gshader -o bin/basic_dx12.gsb -b dx12
```
