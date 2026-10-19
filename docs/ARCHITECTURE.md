# GPU Engine Architecture

## Scope

GPU Engine is a portable C++17 foundation for GPU compute and CAD rendering. It is Blackwell-inspired: it models hardware capability discovery, explicit memory resources, compute dispatch, and acceleration structures without attempting to reproduce NVIDIA proprietary hardware or firmware.

## Layers

```text
CAD / Scene
    |
    +-- cad_render::prepare_mesh()
    +-- CPU reference BVH
    |
Backend-neutral resource and compute APIs
    |
    +-- memory::Buffer
    +-- compute::ComputeRuntime
    |
Backend adapters
    |
    +-- Vulkan
    +-- Direct3D 12 (planned)
    +-- Metal (planned)
    +-- WebGPU (planned)
```

## Vulkan boundary

The Vulkan SDK is optional. CMake detects it with `find_package(Vulkan QUIET)`. When available, the backend creates a Vulkan instance and enumerates physical devices, queue families, vendor/device IDs, and device-local memory. When unavailable, the library remains buildable and returns a clear status from the Vulkan adapter.

## CAD path

`cad_render::prepare_mesh` converts a validated CAD mesh into contiguous position and index buffers. This keeps CAD topology independent from graphics APIs and provides a clean handoff point for native Vulkan buffers later.

## Compute shader contract

`shaders/cad_transform.comp.glsl` defines a 256-thread workgroup transform pass. Binding 0 contains source positions, binding 1 contains transformed positions, and a push constant supplies the model-view-projection matrix.

The shader remains source rather than checked-in SPIR-V so the repository does not depend on a particular shader compiler version.

## Validation strategy

The CPU/reference path remains the correctness oracle. CI validates CAD geometry, CPU BVH construction, backend creation, graceful Vulkan absence, buffer semantics, and CAD mesh upload preparation. A future Vulkan compute test should compare GPU output against the CPU reference within defined floating-point tolerances.
