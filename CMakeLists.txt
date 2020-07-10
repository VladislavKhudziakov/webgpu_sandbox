cmake_minimum_required(VERSION 3.16)
project(wgpu_sandbox)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/*.mm)

add_executable(wgpu_sandbox main.cpp ${SOURCES})

target_include_directories(wgpu_sandbox PRIVATE ${CMAKE_CURRENT_LIST_DIR}/third/glm)
target_include_directories(wgpu_sandbox PRIVATE ${CMAKE_CURRENT_LIST_DIR}/third/wgpu-native/ffi)
target_include_directories(wgpu_sandbox PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/third/CMakeRust/cmake")

enable_language(Rust)
include(CMakeCargo)

file(TOUCH ${CMAKE_CURRENT_LIST_DIR}/third/wgpu-native/CMakeLists.txt)
file(READ ${CMAKE_CURRENT_LIST_DIR}/wgpuCmakeSources.txt WGPU_CMAKE)
file(WRITE ${CMAKE_CURRENT_LIST_DIR}/third/wgpu-native/CMakeLists.txt ${WGPU_CMAKE})

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third/glfw)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third/glslang)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/third/wgpu-native)

set(APPLE_FRAMEWORKS "-framework Cocoa" "-framework CoreVideo" "-framework IOKit" "-framework QuartzCore")

target_link_libraries(
        wgpu_sandbox
        PRIVATE
        ${APPLE_FRAMEWORKS}
        wgpu-native
        glfw
        glslang
        SPIRV
        glslang-default-resource-limits)
