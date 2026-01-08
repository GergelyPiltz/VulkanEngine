cmake_minimum_required(VERSION 3.20)

project(VulkanEngine
    VERSION 1.0
    LANGUAGES CXX
)

# ----------------------------------
# Global configuration
# ----------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force x64 on Visual Studio
if (MSVC)
    if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "This project must be built as x64")
    endif()
endif()

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
# Application
# ----------------------------------
add_executable(app
    src/main.cpp
)

target_link_libraries(app
    PRIVATE
        glfw
)

# ----------------------------------
# Windows system libraries required by GLFW
# ----------------------------------
if (WIN32)
    target_link_libraries(app
        PRIVATE
            user32
            gdi32
            shell32
            winmm
    )
endif()
