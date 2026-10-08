# CMakeLists.txt
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)
project(BasisShaderCompiler
        VERSION 1.0.0
        DESCRIPTION "C++20 multi-backend shader compiler"
        LANGUAGES CXX
)
# ----------------------------------------------------------------------------
option(BSHC_BUILD_EXECUTABLE "Build the BSHC command-line tool" ${PROJECT_IS_TOP_LEVEL})
option(BSHC_BUILD_TESTS      "Build test suite"                 ${PROJECT_IS_TOP_LEVEL})
# ----------------------------------------------------------------------------
if (PROJECT_IS_TOP_LEVEL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif ()
# ----------------------------------------------------------------------------
add_library(bshc_core STATIC
            bshc/src/Lexer.cpp
            bshc/src/Preprocessor.cpp
            bshc/src/Compiler.cpp
            bshc/src/BinaryPackager.cpp
            bshc/src/platform/opengl/GLCompiler.cpp
            bshc/src/platform/vulkan/VulkanCompiler.cpp
            bshc/src/platform/dx12/DX12Compiler.cpp
            bshc/include/Log.hpp
)
add_library(bshc::bshc_core ALIAS bshc_core)
target_compile_features(bshc_core PUBLIC cxx_std_20)
target_include_directories(bshc_core
                           PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/bshc/include
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bshc/src
)
# ----------------------------------------------------------------------------
include(RenderBackend.cmake)
target_link_libraries(bshc_core PUBLIC Vulkan::Shaderc spirv-cross-hlsl)
# ----------------------------------------------------------------------------
if (BSHC_BUILD_EXECUTABLE)
    add_executable(BSHC
                   bshc/src/BSH.cpp
    )
    add_executable(bshc::bshc ALIAS BSHC)
    target_link_libraries(BSHC PRIVATE bshc_core)
endif ()
# ----------------------------------------------------------------------------
if (BSHC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()
# ----------------------------------------------------------------------------
