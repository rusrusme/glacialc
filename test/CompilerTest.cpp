//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

#include "BinaryPackager.hpp"
#include "Compiler.hpp"
#include "Preprocessor.hpp"

using namespace bshc;

constexpr std::string_view sample_shader = R"(
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
)";

static void verify_and_print_binary(const std::string& filepath, RenderBackend expected_backend) {
    std::ifstream file(filepath, std::ios::binary);
    assert(file.is_open() && "Failed to open output binary file");

    BSHHeader header{};
    file.read(reinterpret_cast<char*>(&header), sizeof(BSHHeader));

    std::cout << "\n========================================\n";
    std::cout << "Inspecting Binary File: " << filepath << "\n";
    std::cout << "========================================\n";
    std::cout << "Signature   : 0x" << std::hex << header.signature << std::dec
        << (header.signature == BSH_SIGNATURE ? " (VALID BSHB)" : " (INVALID)") << "\n";
    std::cout << "Version     : " << header.version << "\n";
    std::cout << "Backend ID  : " << static_cast<int>(header.backend) << "\n";
    std::cout << "Stage Count : " << static_cast<int>(header.stage_count) << "\n";

    assert(header.signature == BSH_SIGNATURE);
    assert(header.backend == expected_backend);
    assert(header.stage_count == 2);

    for (uint8_t i = 0; i < header.stage_count; ++i) {
        BSHStageHeader stage_hdr{};
        file.read(reinterpret_cast<char*>(&stage_hdr), sizeof(BSHStageHeader));

        std::cout << "  [Stage " << static_cast<int>(i) << " Header]\n";
        std::cout << "    Type   : " << static_cast<int>(stage_hdr.stage_type) << "\n";
        std::cout << "    Offset : " << stage_hdr.data_offset << " bytes\n";
        std::cout << "    Size   : " << stage_hdr.data_size << " bytes\n";

        assert(stage_hdr.data_size > 0 && "Stage payload cannot be empty");
    }
}

static void test_backend(RenderBackend backend, const std::string& output_filename) {
    Preprocessor preprocessor;
    ParsedData parsed = preprocessor.process(sample_shader);
    assert(parsed.success && "Preprocessing failed");

    parsed.header.backend = backend;

    Unique<Compiler> compiler = Compiler::create(backend);
    assert(compiler != nullptr && "Compiler creation failed");

    CompilerOutput compiled = compiler->compile(parsed);
    if (!compiled.success) {
        std::cerr << "Compilation failed for backend " << static_cast<int>(backend)
            << ": " << compiled.error_message << "\n";
        assert(false);
    }

    auto binary = BinaryPackager::package_binary(parsed.header, compiled.stages);
    bool saved = BinaryPackager::save(output_filename, binary);
    assert(saved && "Failed to save binary file");

    verify_and_print_binary(output_filename, backend);
}

int main() {
    std::cout << "Starting Multi-Backend Shader Pipeline Tests...\n";

    test_backend(RenderBackend::OpenGL, "test_shader_gl.bshb");
    test_backend(RenderBackend::Vulkan, "test_shader_vk.bshb");

    #if defined(_WIN32)
    test_backend(RenderBackend::DX12, "test_shader_dx12.bshb");
    #endif

    std::cout << "\nAll Backend Shader Compiler Pipeline Tests Passed Successfully!\n";
    return 0;
}
