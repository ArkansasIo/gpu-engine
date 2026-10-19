#include "gpu_engine/acceleration.hpp"
#include "gpu_engine/gpu_engine.hpp"
#include "gpu_engine/cad.hpp"
#include "gpu_engine/gpu_compute.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {
bool require(bool condition, const char* message) {
    if (!condition) std::cerr << "FAILED: " << message << '\n';
    return condition;
}
}

int main() {
    bool ok = true;
    const auto info = gpu_engine::get_engine_info();
    ok &= require(!info.name.empty(), "engine name must not be empty");

    const auto* vulkan_label = gpu_engine::backend_name(gpu_engine::Backend::Vulkan);
    ok &= require(vulkan_label != nullptr && std::string(vulkan_label) == "Vulkan",
                  "Vulkan backend label");

    gpu_engine::cad::CadEngine cad;
    cad.add_box("Test Bracket", {-1,-1,-0.5}, {1,1,0.5});
    cad.add_cylinder("Test Shaft", 0.25, 2.0, 24);
    const auto& assembly = cad.assembly();
    ok &= require(assembly.parts.size() == 2, "two CAD parts should exist");
    std::string error;
    for (const auto& part : assembly.parts) {
        ok &= require(cad.validate_mesh(part.mesh, &error), "generated mesh validates");
        if (!error.empty()) std::cerr << error << '\n';
    }
    ok &= require(assembly.parts[0].mesh.triangles.size() == 12, "box has 12 triangles");

    gpu_engine::accel::Bvh bvh;
    bvh.build(assembly.parts[0].mesh);
    ok &= require(bvh.node_count() > 0, "BVH has nodes for a non-empty mesh");
    const auto hit = bvh.intersect(assembly.parts[0].mesh, {{0,0,3},{0,0,-1}});
    ok &= require(hit.hit, "ray through box should intersect");
    ok &= require(std::abs(hit.t - 2.5) < 1e-9, "nearest box hit distance should be 2.5");
    const auto miss = bvh.intersect(assembly.parts[0].mesh, {{3,3,3},{0,0,-1}});
    ok &= require(!miss.hit, "ray outside box should miss");
    const auto zero_ray = bvh.intersect(assembly.parts[0].mesh, {{0,0,3},{0,0,0}});
    ok &= require(!zero_ray.hit, "zero direction ray should miss safely");

    gpu_engine::cad::Mesh invalid_mesh;
    invalid_mesh.vertices.push_back({{0,0,0}});
    invalid_mesh.triangles.push_back({0,1,2});
    gpu_engine::accel::Bvh invalid_bvh;
    invalid_bvh.build(invalid_mesh);
    ok &= require(invalid_bvh.node_count() == 0, "invalid triangles are skipped");

    gpu_engine::compute::Device device;
    device.name = "Blackwell-Inspired Reference GPU";
    device.compute_units = 128;
    device.lanes_per_wave = 32;
    gpu_engine::compute::ComputeRuntime runtime(device);
    std::vector<std::size_t> ids;
    runtime.dispatch({1000, 128}, [&](std::size_t id) { ids.push_back(id); });
    ok &= require(ids.size() == 1000, "dispatch executes all elements");
    ok &= require(!ids.empty() && ids.front() == 0 && ids.back() == 999,
                  "dispatch preserves element ID coverage");

    std::vector<std::size_t> none;
    runtime.dispatch({10, 0}, [&](std::size_t id) { none.push_back(id); });
    ok &= require(none.size() == 10, "zero workgroup size safely defaults to one");

    if (!ok) return EXIT_FAILURE;
    std::cout << "GPU CAD engine smoke test passed: " << assembly.parts.size()
              << " parts, " << bvh.node_count() << " BVH nodes, "
              << ids.size() << " compute elements\n";
    return EXIT_SUCCESS;
}
