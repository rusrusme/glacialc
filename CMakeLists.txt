# CMakeLists.txt
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 4.3)
# ----------------------------------------------------------------------------
project(BasisShaderCompiler LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
# ----------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# ----------------------------------------------------------------------------
add_library(bshc_core STATIC
            bshc/src/Lexer.cpp
            bshc/include/Lexer.hpp
            bshc/include/BSH.hpp
            bshc/src/Preprocessor.cpp
            bshc/include/Preprocessor.hpp
            bshc/include/Compiler.hpp
            bshc/src/platform/OpenGL/GLCompiler.cpp
            bshc/include/platform/OpenGL/GLCompiler.hpp
            bshc/src/platform/vulkan/VulkanCompiler.cpp
            bshc/include/platform/vulkan/VulkanCompiler.hpp
            bshc/src/platform/dx12/DX12Compiler.cpp
            bshc/include/platform/dx12/DX12Compiler.hpp
            bshc/src/BinaryPackager.cpp
            bshc/include/BinaryPackager.hpp
            bshc/src/Compiler.cpp
            bshc/include/Utility.hpp
)
target_include_directories(bshc_core
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/bshc/include
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bshc/src
)
# ----------------------------------------------------------------------------
include(RenderBackend.cmake)
target_link_libraries(bshc_core PUBLIC Vulkan::Shaderc)
# ----------------------------------------------------------------------------
add_executable(BSHC bshc/src/BSH.cpp
               bshc/include/Log.hpp)
# ----------------------------------------------------------------------------
target_link_libraries(BSHC PRIVATE bshc_core)
# ----------------------------------------------------------------------------
add_subdirectory(test)
# ----------------------------------------------------------------------------
