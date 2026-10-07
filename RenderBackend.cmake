# RenderBackend.cmake
# ----------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)
# ----------------------------------------------------------------------------
if(TARGET Vulkan::shaderc_combined)
    add_library(Vulkan::Shaderc ALIAS Vulkan::shaderc_combined)
elseif(TARGET Vulkan::shaderc_shared)
    add_library(Vulkan::Shaderc ALIAS Vulkan::shaderc_shared)
else()
    find_library(SHADERC_LIB
                 NAMES shaderc_shared shaderc_combined
                 HINTS $ENV{VULKAN_SDK}/Lib
    )
    add_library(Vulkan::Shaderc UNKNOWN IMPORTED)
    set_target_properties(Vulkan::Shaderc PROPERTIES
                          IMPORTED_LOCATION "${SHADERC_LIB}"
                          INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}"
    )
endif()
# ----------------------------------------------------------------------------
if (WIN32)
    target_compile_definitions(bshc_core PUBLIC RENDER_API_DX12)
    target_link_libraries(bshc_core PUBLIC d3d12.lib dxgi.lib dxguid.lib)
endif ()
# ----------------------------------------------------------------------------
