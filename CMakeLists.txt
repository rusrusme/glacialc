# CMakeLists.txt
cmake_minimum_required(VERSION 4.3)

project(BasisShaderCompiler LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(Dependencies.cmake)

add_library(bshc_core STATIC
            bshc/src/Lexer.cpp
            bshc/include/Lexer.hpp
            bshc/include/BSH.hpp
            bshc/src/Preprocessor.cpp
            bshc/include/Preprocessor.hpp
            bshc/src/Compiler.cpp
            bshc/include/Compiler.hpp
)

target_include_directories(bshc_core
                           PUBLIC  ${CMAKE_SOURCE_DIR}/bshc/include
                           PRIVATE ${CMAKE_SOURCE_DIR}/bshc/src
)

target_precompile_headers(bshc_core PRIVATE bshc/src/bshc_pch.hpp)
target_link_libraries(bshc_core PUBLIC Vulkan::Shaderc)

add_executable(BSHC bshc/src/BSH.cpp)
target_link_libraries(BSHC PRIVATE bshc_core)

add_subdirectory(test)