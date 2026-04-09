cmake_minimum_required(VERSION 3.20)

project(VulkanEngineProject
    VERSION 1.0
    LANGUAGES CXX
)

# ----------------------------------
# Global configuration
# ----------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force x64 on Visual Studio
if (MSVC)
    if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "This project must be built as x64")
    endif()
endif()

# ----------------------------------
# Application
# ----------------------------------
add_executable(VulkanEngine
    "src/main.cpp"
    "src/window.cpp"
    "src/pipeline.cpp"
    "src/vulkan_app.cpp"
    "src/device.cpp" 
    "src/swapchain.cpp"
    "src/model.cpp"
    "src/renderer.cpp"
    "src/render_system.cpp"
    "src/camera.cpp"
    "src/keyboard_movement_controller.cpp" 
    "src/game_object.cpp"
    "src/buffer.cpp" 
    "src/descriptors.cpp"
)

include_directories(
    "include"
)

target_link_libraries(VulkanEngine
    PRIVATE
        glm
        glfw
        Vulkan::Vulkan
)

# ----------------------------------
# Vulkan configuration (SDK should be installed and environment variable present)
# ----------------------------------
find_package(Vulkan REQUIRED)

# ----------------------------------
# GLM configuration
# ----------------------------------
add_subdirectory(external/glm)

# ----------------------------------
# GLFW configuration (STATIC)
# ----------------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Static library
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Prevent GLFW from overriding CRT flags
set(GLFW_USE_HYBRID_HPG OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

# ----------------------------------
# Windows system libraries required by GLFW
# ----------------------------------
if (WIN32)
    target_link_libraries(VulkanEngine
        PRIVATE
            user32
            gdi32
            shell32
            winmm
    )
endif()

# ----------------------------------
# Compile and Copy shaders
# ----------------------------------
find_program(GLSLC glslc REQUIRED)

set(SHADER_SRC_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(SHADER_BIN_DIR "${CMAKE_BINARY_DIR}/shaders")

file(GLOB_RECURSE SHADER_FILES
    CONFIGURE_DEPENDS
    "${SHADER_SRC_DIR}/*.vert"
    "${SHADER_SRC_DIR}/*.frag"
    "${SHADER_SRC_DIR}/*.comp"
    "${SHADER_SRC_DIR}/*.geom"
    "${SHADER_SRC_DIR}/*.tesc"
    "${SHADER_SRC_DIR}/*.tese"
)

set(SHADER_FLAGS "")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SHADER_FLAGS "-g")
else()
    set(SHADER_FLAGS "-O")
endif()

set(SPIRV_OUTPUTS)

foreach(shader ${SHADER_FILES})
    file(RELATIVE_PATH REL_PATH ${SHADER_SRC_DIR} ${shader})

    get_filename_component(NAME_WE ${shader} NAME_WE)
    set(SPIRV "${SHADER_BIN_DIR}/${NAME_WE}.spv")

    get_filename_component(SPIRV_DIR ${SPIRV} DIRECTORY)

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SPIRV_DIR}
        COMMAND ${GLSLC} ${shader} -o ${SPIRV} ${SHADER_FLAGS}
        DEPENDS ${shader}
        COMMENT "Compiling shader ${REL_PATH}"
        VERBATIM
    )

    list(APPEND SPIRV_OUTPUTS ${SPIRV})
endforeach()

add_custom_target(shaders DEPENDS ${SPIRV_OUTPUTS})

add_dependencies(VulkanEngine shaders)

add_custom_command(TARGET VulkanEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SHADER_BIN_DIR}"
        "$<TARGET_FILE_DIR:VulkanEngine>/shaders"
)

# ----------------------------------
# Copy Textures
# ----------------------------------
add_custom_command(TARGET VulkanEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/textures"
        "$<TARGET_FILE_DIR:VulkanEngine>/textures"
)

# ----------------------------------
# Copy Models
# ----------------------------------
add_custom_command(TARGET VulkanEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/models"
        "$<TARGET_FILE_DIR:VulkanEngine>/models"
)