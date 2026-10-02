//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <iostream>
#include <fstream>
#include <filesystem>

#include "Preprocessor.hpp"

using namespace bshc;

int main() {
    namespace fs = std::filesystem;

    fs::path temp_dir = fs::temp_directory_path() / "bshc_circular_test";
    fs::create_directories(temp_dir);

    fs::path file_a = temp_dir / "file_a.glsl";
    fs::path file_b = temp_dir / "file_b.glsl";
    fs::path main_shader_path = temp_dir / "main.bsh";

    {
        std::ofstream out_a(file_a);
        out_a << "// Header A\n#include \"file_b.glsl\"\n";
    }

    {
        std::ofstream out_b(file_b);
        out_b << "// Header B\n#include \"file_a.glsl\"\n";
    }

    std::string root_source = R"(
        @depth_test = true

        #include "file_a.glsl"

        #vert
        void main() {}
    )";

    {
        std::ofstream out_main(main_shader_path);
        out_main << root_source;
    }

    Preprocessor preprocessor(temp_dir);
    ParseResult result = preprocessor.process(root_source, main_shader_path);

    fs::remove_all(temp_dir);

    if (!result.success) {
        std::cout << "Circular include caught successfully!\n";
        std::cout << "Error message: " << result.error_message << "\n";
        return 0;
    }

    std::cerr << "Circular include was not detected!\n";
    return 1;
}
