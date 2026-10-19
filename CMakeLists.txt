cmake_minimum_required(VERSION 3.20)

project(GPUEngine
    VERSION 0.2.0
    DESCRIPTION "Blackwell-inspired compute architecture and GPU CAD engine"
    LANGUAGES CXX
)

option(GPU_ENGINE_BUILD_TESTS "Build GPU Engine tests" ON)

add_library(gpu_engine
    src/gpu_engine.cpp
    src/math.cpp
    src/cad.cpp
    src/gpu_compute.cpp
)
add_library(GPUEngine::gpu_engine ALIAS gpu_engine)

target_compile_features(gpu_engine PUBLIC cxx_std_17)
target_include_directories(gpu_engine PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)

target_compile_definitions(gpu_engine PUBLIC GPU_ENGINE_API_VERSION_MAJOR=0 GPU_ENGINE_API_VERSION_MINOR=2)

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
endif()
