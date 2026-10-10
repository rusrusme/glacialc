//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <iostream>
#include <string_view>

#include "../bshc/include/bshc/Preprocessor.hpp"

using namespace bshc;

int main() {
    constexpr std::string_view sample_shader = R"(
        // Shared layout across all stages
        layout(binding = 0) uniform Camera {
            mat4 view_proj;
        };

        #vert
        layout(location = 0) in vec3 a_Pos;
        void main() {
            gl_Position = view_proj * vec4(a_Pos, 1.0);
        }

        #frag
        layout(location = 0) out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";

    Preprocessor preprocessor;
    auto [header, stages, success, error_message] = preprocessor.process(sample_shader);

    if (!success) {
        std::cerr << "Preprocessing failed: " << error_message << "\n";
        return 1;
    }

    std::cout << "--- BSHHeader State ---\n";
    std::cout << "Stage Count : " << static_cast<int>(header.stage_count) << "\n";

    for (size_t i = 0; i < stages.size(); ++i) {
        std::cout << "--- Stage " << i << " (Type: " << static_cast<int>(stages[i].stage) << ") GLSL Source ---\n";
        std::cout << stages[i].source << "\n";
    }

    return 0;
}
