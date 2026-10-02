# CMakeLists.txt
cmake_minimum_required(VERSION 4.3)

project(BasisShaderCompiler LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(BSH_TARGET_API "OPENGL" CACHE STRING "Target graphics API for shader compilation")
set_property(CACHE BSH_TARGET_API PROPERTY STRINGS "OPENGL" "VULKAN")

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
                           PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/bshc/include
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bshc/src
)

if (BSH_TARGET_API STREQUAL "OPENGL")
    target_compile_definitions(bshc_core PUBLIC BSH_TARGET_OPENGL)
    message(STATUS "BSHCompiler target environment: OPENGL (OpenGL 4.5)")
elseif (BSH_TARGET_API STREQUAL "VULKAN")
    target_compile_definitions(bshc_core PUBLIC BSH_TARGET_VULKAN)
    message(STATUS "BSHCompiler target environment: VULKAN (Vulkan 1.3)")
else ()
    message(FATAL_ERROR "Unsupported BSH_TARGET_API: ${BSH_TARGET_API}")
endif ()

target_link_libraries(bshc_core PUBLIC Vulkan::Shaderc)

add_executable(BSHC bshc/src/BSH.cpp)
target_link_libraries(BSHC PRIVATE bshc_core)

add_subdirectory(test)