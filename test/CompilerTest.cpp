//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "../bshc/include/bshc/BinaryPackager.hpp"
#include "../bshc/include/bshc/Compiler.hpp"
#include "../bshc/include/bshc/Preprocessor.hpp"
#include "bshc/Utility.hpp"

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

constexpr std::string_view compute_shader = R"(
    layout(std430, binding = 0) buffer InputBuffer {
        float data_in[];
    };

    layout(std430, binding = 1) buffer OutputBuffer {
        float data_out[];
    };

    #comp
    layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

    void main() {
        uint index = gl_GlobalInvocationID.x;
        data_out[index] = data_in[index] * 2.0;
    }
)";

constexpr std::string_view invalid_syntax_shader = R"(
    #vert
    void main() {
        invalid_type x = vec3(1.0)
        gl_Position = x;
    }
)";

constexpr std::string_view comment_trap_shader = R"(
    layout(binding = 0) uniform GlobalData {
        mat4 u_VP;
    };

    // Fake directive in single line comment: #frag
    /*
       Fake directive in block comment: #geom
       layout(location = 0) in vec4 dummy;
    */

    #vert
    layout(location = 0) in vec3 a_Pos;
    layout(location = 0) out vec3 v_Pos;

    void main() {
        v_Pos = a_Pos;
    }

    #frag
    layout(location = 0) in vec3 v_Pos;
    layout(location = 0) out vec4 FragColor;

    void main() {
        FragColor = vec4(v_Pos, 1.0);
    }
)";

static void verify_and_print_binary(const std::string& filepath, RenderBackend expected_backend, uint8_t expected_stage_count) {
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
    assert(header.stage_count == expected_stage_count);

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
    std::cout << "\n[TEST] Pipeline Standard Shader (" << to_string(backend) << ")...\n";
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

    verify_and_print_binary(output_filename, backend, 2);
}

static void test_compute_shader(RenderBackend backend, const std::string& output_filename) {
    std::cout << "\n[TEST] Compute Shader Pipeline (" << to_string(backend) << ")...\n";
    Preprocessor preprocessor;
    ParsedData parsed = preprocessor.process(compute_shader);
    assert(parsed.success && "Compute preprocessing failed");

    parsed.header.backend = backend;

    Unique<Compiler> compiler = Compiler::create(backend);
    assert(compiler != nullptr);

    CompilerOutput compiled = compiler->compile(parsed);
    if (!compiled.success) {
        std::cerr << "Compute compilation failed for backend " << static_cast<int>(backend)
            << ": " << compiled.error_message << "\n";
        assert(false);
    }

    auto binary = BinaryPackager::package_binary(parsed.header, compiled.stages);
    bool saved = BinaryPackager::save(output_filename, binary);
    assert(saved);

    verify_and_print_binary(output_filename, backend, 1);
}

static void test_invalid_syntax(RenderBackend backend) {
    std::cout << "\n[TEST] Invalid Syntax Error Diagnostic (" << to_string(backend) << ")...\n";
    Preprocessor preprocessor;
    ParsedData parsed = preprocessor.process(invalid_syntax_shader);
    assert(parsed.success && "Preprocessing should succeed for invalid syntax test source");

    parsed.header.backend = backend;

    Unique<Compiler> compiler = Compiler::create(backend);
    assert(compiler != nullptr);

    CompilerOutput compiled = compiler->compile(parsed);
    assert(!compiled.success && "Compiler must fail on invalid syntax");
    assert(!compiled.error_message.empty() && "Compiler error message must not be empty");

    std::cout << "Caught expected compilation failure:\n" << compiled.error_message << "\n";
}

static void test_circular_dependency() {
    std::cout << "\n[TEST] Circular #include Dependency Guard...\n";
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path() / "bshc_test_circ";
    std::filesystem::create_directories(temp_dir);

    std::ofstream(temp_dir / "A_common.bsh") << "#pragma once\nstruct PointLight { vec3 pos; vec3 color; };\n";
    std::ofstream(temp_dir / "B_lighting.bsh") << "#include \"A_common.bsh\"\n#include \"C_main.bsh\"\n";
    std::ofstream(temp_dir / "C_main.bsh") << "#include \"A_common.bsh\"\n#include \"B_lighting.bsh\"\n#vert\nvoid main(){}\n";

    std::filesystem::path main_file = temp_dir / "C_main.bsh";
    std::string source;
    {
        std::ifstream file(main_file);
        source = std::string((std::istreambuf_iterator<char>(file)), {});
    } // file wird hier sauber geschlossen!

    Preprocessor preprocessor(temp_dir);
    ParsedData parsed = preprocessor.process(source, main_file);

    assert(!parsed.success && "Circular dependency must fail preprocessing");
    assert(parsed.error_message.find("Circular #include") != std::string::npos);

    std::cout << "Caught expected circular dependency error:\n" << parsed.error_message << "\n";

    std::error_code ec;
    std::filesystem::remove_all(temp_dir, ec);
}

static void test_comment_trap() {
    std::cout << "\n[TEST] Comment Directive Trap Handling...\n";
    Preprocessor preprocessor;
    ParsedData parsed = preprocessor.process(comment_trap_shader);

    assert(parsed.success && "Comment trap preprocessing failed");
    assert(parsed.stages.size() == 2 && "Preprocessor should isolate exactly 2 stages (#vert, #frag)");
    assert(parsed.stages[0].stage == ShaderStage::Vertex);
    assert(parsed.stages[1].stage == ShaderStage::Fragment);

    std::cout << "Comment trap test passed: Directives inside comments ignored successfully.\n";
}

static void test_header_only_shader(RenderBackend backend) {
    std::cout << "\n[TEST] Header-Only File Handling (" << to_string(backend) << ")...\n";
    constexpr std::string_view header_only = "layout(binding = 0) uniform Global { mat4 u_VP; };";

    Preprocessor preprocessor;
    ParsedData parsed = preprocessor.process(header_only);

    assert(parsed.stages.empty() && "No stages should be parsed for header-only input");

    parsed.header.backend = backend;
    Unique<Compiler> compiler = Compiler::create(backend);
    CompilerOutput compiled = compiler->compile(parsed);

    assert(!compiled.success && "Compilation must fail when zero stages are present");
    std::cout << "Caught expected empty stage error:\n" << compiled.error_message << "\n";
}

int main() {
    std::cout << "Starting Multi-Backend & Edge Case Pipeline Tests...\n";

    // 1. Standard Graphics Shader Tests
    test_backend(RenderBackend::OpenGL, "test_shader_gl.bshb");
    test_backend(RenderBackend::Vulkan, "test_shader_vk.bshb");

    // 2. Compute Shader Pipeline Tests
    test_compute_shader(RenderBackend::OpenGL, "test_compute_gl.bshb");
    test_compute_shader(RenderBackend::Vulkan, "test_compute_vk.bshb");

    // 3. Error Diagnostic Tests
    test_invalid_syntax(RenderBackend::OpenGL);
    test_invalid_syntax(RenderBackend::Vulkan);

    // 4. Preprocessor Edge Cases
    test_circular_dependency();
    test_comment_trap();
    test_header_only_shader(RenderBackend::OpenGL);

    #if defined(_WIN32)
    test_backend(RenderBackend::DX12, "test_shader_dx12.bshb");
    test_compute_shader(RenderBackend::DX12, "test_compute_dx12.bshb");
    test_invalid_syntax(RenderBackend::DX12);
    test_header_only_shader(RenderBackend::DX12);
    #endif

    std::cout << "\nAll Pipeline and Edge Case Tests Executed Successfully!\n";
    return 0;
}
