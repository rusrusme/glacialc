# CMakeLists.txt
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)
project(glacialc
        VERSION 1.0.0
        DESCRIPTION "C++20 multi-backend shader compiler"
        LANGUAGES CXX
)
# ----------------------------------------------------------------------------
option(GLACIALC_BUILD_EXECUTABLE "Build the glacialc command-line tool" ${PROJECT_IS_TOP_LEVEL})
option(GLACIALC_BUILD_TESTS "Build test suite" ${PROJECT_IS_TOP_LEVEL})
# ----------------------------------------------------------------------------
if (PROJECT_IS_TOP_LEVEL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif ()
# ----------------------------------------------------------------------------
add_library(glacialc_core STATIC
            glacialc/src/Lexer.cpp
            glacialc/src/Preprocessor.cpp
            glacialc/src/Compiler.cpp
            glacialc/src/BinaryPackager.cpp
            glacialc/src/platform/opengl/GLCompiler.cpp
            glacialc/src/platform/vulkan/VulkanCompiler.cpp
            glacialc/src/platform/dx12/DX12Compiler.cpp
            glacialc/include/glacialc/Log.hpp
            glacialc/src/GShaderCompiler.cpp
            glacialc/include/glacialc/GShaderCompiler.hpp
)
add_library(glacialc::glacialc_core ALIAS glacialc_core)
target_compile_features(glacialc_core PUBLIC cxx_std_20)
target_include_directories(glacialc_core
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/glacialc/include
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/glacialc/src
)
# ----------------------------------------------------------------------------
include(RenderBackend.cmake)
target_link_libraries(glacialc_core PUBLIC Vulkan::Shaderc spirv-cross-hlsl)
# ----------------------------------------------------------------------------
if (GLACIALC_BUILD_EXECUTABLE)
    add_executable(glacialc
                   glacialc/src/main.cpp
    )
    add_executable(glacialc::glacialc ALIAS glacialc)
    target_link_libraries(glacialc PRIVATE glacialc_core)
endif ()
# ----------------------------------------------------------------------------
if (GLACIALC_BUILD_EXECUTABLE)
    enable_testing()
    add_subdirectory(test)
endif ()
# ----------------------------------------------------------------------------
