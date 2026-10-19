# GPU Engine — 3D CAD Compute Platform

A C++17, backend-neutral compute and 3D CAD foundation inspired by the architectural ideas behind modern GPU accelerators such as NVIDIA Blackwell: massive parallel dispatch, explicit device capabilities, memory-aware workloads, geometry acceleration, and a separation between the engine API and hardware backends.

**Important:** this project is an independent software implementation. It does not contain NVIDIA proprietary firmware, drivers, CUDA source, or Blackwell hardware IP, and it does not claim Blackwell performance.

## What exists now

- Procedural CAD primitives: boxes and cylinders.
- Mesh and AABB representation with deterministic validation.
- Backend-neutral compute runtime with GPU-style workgroup dispatch.
- Device model with compute-unit, wave/warp-width, and memory capability fields.
- Public C++ API designed so real Vulkan, CUDA, Direct3D 12, Metal, or WebGPU adapters can be added later.
- CMake build and automated Linux/Windows/macOS CI.

## Architecture

```text
CAD Application
      |
      v
+-----------------------+
| GPU CAD Engine API    |
| scene / mesh / math   |
+-----------+-----------+
            |
     +------+------+
     |             |
     v             v
Geometry         Compute Runtime
BVH / tessell.   dispatch / kernels
     |             |
     +------+------+
            v
     Backend Adapter Layer
   Vulkan / CUDA / D3D12 / Metal
            |
            v
       Physical GPU
```

The current compute runtime is a **reference software backend**. It provides the same logical dispatch contract a hardware backend can implement, allowing CAD algorithms to be tested without requiring a particular GPU.

## CAD engine direction

The target is a professional CAD-oriented compute stack rather than a game renderer. Planned workloads include:

- B-rep and NURBS geometry.
- Parametric feature history and constraints.
- GPU tessellation of analytic surfaces.
- Mesh repair, normals, curvature, and decimation.
- BVH / acceleration structures for selection, interference, ray queries, and visualization.
- Boolean and distance-field operations.
- Massive assembly processing.
- GPU-assisted simulation kernels.
- Large-model streaming and out-of-core memory management.

## Blackwell-inspired software model

The software architecture uses accelerator concepts without attempting to reproduce proprietary hardware:

| Concept | GPU Engine implementation |
|---|---|
| Parallel execution | Workgroup/element dispatch API |
| GPU capability model | `compute::Device` |
| Warp-like width | `lanes_per_wave` |
| Compute clusters | `compute_units` |
| Explicit workload size | `compute::Dispatch` |
| Geometry acceleration | CAD AABB/BVH layer |
| Hardware abstraction | Backend-neutral public API |
| CPU fallback | Reference compute runtime |

These are software abstractions, not measurements of an NVIDIA GPU.

## Repository layout

```text
gpu-engine/
├── CMakeLists.txt
├── README.md
├── include/gpu_engine/
│   ├── gpu_engine.hpp       # engine/version/backend API
│   ├── math.hpp             # vectors and matrices
│   ├── cad.hpp              # CAD scene and procedural geometry
│   ├── gpu_compute.hpp      # accelerator-neutral dispatch
│   └── acceleration.hpp     # geometry acceleration API
├── src/
│   ├── gpu_engine.cpp
│   ├── math.cpp
│   ├── cad.cpp
│   └── gpu_compute.cpp
├── tests/
│   └── smoke_test.cpp
└── .github/workflows/
    └── ci.yml
```

## Build

Requirements: CMake 3.20+ and a C++17 compiler (GCC, Clang, or MSVC).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Roadmap

### Phase 1 — Core CAD compute
- [x] Math primitives
- [x] Procedural mesh primitives
- [x] Mesh validation
- [x] Reference parallel dispatch
- [ ] BVH implementation
- [ ] GPU buffer abstraction

### Phase 2 — Real acceleration backend
- [ ] Vulkan compute backend
- [ ] Optional CUDA backend
- [ ] Descriptor/resource management
- [ ] GPU memory allocator
- [ ] asynchronous command queues
- [ ] profiling and GPU timestamp support

### Phase 3 — CAD kernel
- [ ] NURBS curves and surfaces
- [ ] B-rep topology
- [ ] constraints and parametric features
- [ ] robust booleans
- [ ] tessellation cache
- [ ] assembly graph

### Phase 4 — Professional CAD viewport
- [ ] GPU selection/picking
- [ ] physically based viewport
- [ ] section/clipping planes
- [ ] measurement tools
- [ ] hidden-line rendering
- [ ] large-assembly streaming

### Phase 5 — Simulation
- [ ] finite-element data model
- [ ] particle/mesh kernels
- [ ] collision and contact acceleration
- [ ] thermal/structural solver interfaces

## Status

Version 0.2.0. The repository is a functional foundation, not a finished Blackwell-class GPU or commercial CAD kernel. Hardware-specific performance claims require real GPU backends and benchmark validation.

## License

No license has been selected yet. Until one is added, do not assume this code is licensed for redistribution or reuse.
