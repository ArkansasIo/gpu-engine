cmake_minimum_required(VERSION 3.20)

project(GPUEngine
    VERSION 0.3.0
    DESCRIPTION "Blackwell-inspired compute architecture and GPU CAD engine"
    LANGUAGES CXX
)

option(GPU_ENGINE_BUILD_TESTS "Build GPU Engine tests" ON)
option(GPU_ENGINE_ENABLE_VULKAN "Enable Vulkan hardware backend when the SDK is available" ON)

add_library(gpu_engine
    src/gpu_engine.cpp
    src/math.cpp
    src/cad.cpp
    src/gpu_compute.cpp
    src/acceleration.cpp
    src/backend.cpp
    src/vulkan_backend.cpp
    src/buffer.cpp
)
add_library(GPUEngine::gpu_engine ALIAS gpu_engine)

target_compile_features(gpu_engine PUBLIC cxx_std_17)
target_include_directories(gpu_engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(gpu_engine PUBLIC
    GPU_ENGINE_API_VERSION_MAJOR=0
    GPU_ENGINE_API_VERSION_MINOR=3
)

if(GPU_ENGINE_ENABLE_VULKAN)
    find_package(Vulkan QUIET)
    if(Vulkan_FOUND)
        target_compile_definitions(gpu_engine PRIVATE GPU_ENGINE_HAS_VULKAN=1)
        target_link_libraries(gpu_engine PRIVATE Vulkan::Vulkan)
        message(STATUS "GPU Engine: Vulkan backend enabled")
    else()
        message(STATUS "GPU Engine: Vulkan SDK not found; building portable stub backend")
    endif()
endif()

if(MSVC)
    target_compile_options(gpu_engine PRIVATE /W4 /permissive-)
else()
    target_compile_options(gpu_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(GPU_ENGINE_BUILD_TESTS)
    enable_testing()
    add_executable(gpu_engine_smoke_test tests/smoke_test.cpp)
    target_link_libraries(gpu_engine_smoke_test PRIVATE GPUEngine::gpu_engine)
    add_test(NAME gpu_engine_smoke_test COMMAND gpu_engine_smoke_test)

    add_executable(gpu_engine_backend_test tests/backend_test.cpp)
    target_link_libraries(gpu_engine_backend_test PRIVATE GPUEngine::gpu_engine)
    add_test(NAME gpu_engine_backend_test COMMAND gpu_engine_backend_test)

    add_executable(gpu_engine_buffer_test tests/buffer_test.cpp)
    target_link_libraries(gpu_engine_buffer_test PRIVATE GPUEngine::gpu_engine)
    add_test(NAME gpu_engine_buffer_test COMMAND gpu_engine_buffer_test)
endif()
