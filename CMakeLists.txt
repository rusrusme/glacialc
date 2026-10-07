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
include(Dependencies.cmake)
# ----------------------------------------------------------------------------
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
# ----------------------------------------------------------------------------
set(RENDER_API "OPENGL" CACHE STRING "Selects the rendering backend")
set_property(CACHE RENDER_API PROPERTY STRINGS "OPENGL" "VULKAN" "DX12")
if (RENDER_API STREQUAL "OPENGL")
    target_compile_definitions(bshc_core PUBLIC RENDER_API_OPENGL)
    find_package(OpenGL REQUIRED)
elseif (RENDER_API STREQUAL "VULKAN")
    target_compile_definitions(bshc_core PUBLIC RENDER_API_VULKAN)
    find_package(Vulkan REQUIRED)
elseif (RENDER_API STREQUAL "DX12")
    if (NOT WIN32)
        message(FATAL_ERROR "DirectX 12 is only supported on Windows platforms")
    endif ()
    target_compile_definitions(bshc_core PUBLIC RENDER_API_DX12)
    target_link_libraries(bshc_core PUBLIC d3d12.lib dxgi.lib dxguid.lib)
else ()
    message(FATAL_ERROR "Unsupported Render API: ${RENDER_API}")
endif ()
# ----------------------------------------------------------------------------
add_executable(BSHC bshc/src/BSH.cpp
               bshc/include/Log.hpp)
# ----------------------------------------------------------------------------
target_link_libraries(bshc_core PUBLIC Vulkan::Shaderc)
target_link_libraries(BSHC PRIVATE bshc_core)
# ----------------------------------------------------------------------------
add_subdirectory(test)
# ----------------------------------------------------------------------------
