# GPU Engine

A modular GPU computing and graphics-engine foundation for ArkansasIo projects.

> Status: initial scaffold. This repository currently provides project direction and a validation baseline; hardware-specific acceleration backends must be implemented and tested against the target platform.

## Goals

- Provide a backend-neutral API for GPU device discovery, capabilities, resource management, and command submission.
- Separate the public API from backend implementations (for example, Vulkan, Direct3D 12, Metal, or WebGPU).
- Keep CPU-only builds and tests possible where a GPU SDK is unavailable.
- Establish repeatable formatting, static checks, and CI from the start.
- Avoid claiming a backend works until its adapter builds and passes tests on supported hardware.

## Initial layout

```text
gpu-engine/
├── README.md
├── .gitignore
├── CMakeLists.txt
├── include/gpu_engine/
│   └── gpu_engine.hpp
├── src/
│   └── gpu_engine.cpp
├── tests/
│   └── smoke_test.cpp
└── .github/workflows/
    └── ci.yml
```

## Requirements

- CMake 3.20+
- A C++17 compiler (GCC, Clang, or MSVC)
- Python 3 only if extending the optional validation scripts

No vendor GPU SDK is required for the initial CPU-only smoke test.

## Build and test

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
```

## Current API scope

The first milestone is a small, deterministic API and capability model. It is not yet a GPU driver, renderer, shader compiler, or compute runtime. Actual hardware backends should be added behind the public interface and tested on each target operating system and GPU family.

## Roadmap

1. Stabilize the public API and CPU-only tests.
2. Add explicit device/backend interfaces and error reporting.
3. Implement one real backend (Vulkan is a practical cross-platform starting point).
4. Add buffer/image lifetime management, command queues, and synchronization.
5. Add shader and pipeline compilation, then compute and rendering examples.
6. Add backend-specific integration tests on GPU-enabled runners.

## License

No license has been selected yet. Until one is added, do not assume this code is licensed for redistribution or reuse.
