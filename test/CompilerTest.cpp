//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <iostream>
#include <string_view>

#include "Compiler.hpp"
#include "Preprocessor.hpp"

using namespace bshc;

int main() {
    constexpr std::string_view sample_shader = R"(
        @depth_test = true
        @cull_mode = back

        #vert
        #version 450
        layout(location = 0) in vec3 a_Pos;
        void main() {
            gl_Position = vec4(a_Pos, 1.0);
        }

        #frag
        #version 450
        layout(location = 0) out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";

    Preprocessor preprocessor;
    const ParsedData parse_result = preprocessor.process(sample_shader);

    Compiler compiler;
    auto [success, error_message, binary_data] = compiler.compile(parse_result, "pbr_shader.bsh");

    if (!success) {
        std::cerr << "Compilation Failed:\n" << error_message << "\n";
        return 1;
    }

    compiler.compile_to_file(parse_result, "test_shader.bshb", "pbr_shader.bsh");

    std::cout << "Compilation Successful!\n";
    std::cout << "Generated .bshb Payload Size: " << binary_data.size() << " bytes\n";

    // Verify file header signature
    const auto* header = reinterpret_cast<const BSHHeader*>(binary_data.data());
    std::cout << "Header Magic Match: " << (header->signature == BSH_SIGNATURE ? "YES" : "NO") << "\n";
    std::cout << "Header Stage Count: " << static_cast<int>(header->stage_count) << "\n";

    return 0;
}
