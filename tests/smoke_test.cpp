#include "gpu_engine/gpu_engine.hpp"
#include "gpu_engine/cad.hpp"
#include "gpu_engine/gpu_compute.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

int main() {
    const auto info = gpu_engine::get_engine_info();
    if (info.name.empty()) return EXIT_FAILURE;

    const auto* vulkan_label = gpu_engine::backend_name(gpu_engine::Backend::Vulkan);
    if (vulkan_label == nullptr || std::string(vulkan_label) != "Vulkan") return EXIT_FAILURE;

    gpu_engine::cad::CadEngine cad;
    auto& box = cad.add_box("Test Bracket", {-1,-1,-0.5}, {1,1,0.5});
    auto& cylinder = cad.add_cylinder("Test Shaft", 0.25, 2.0, 24);
    std::string error;
    if (!cad.validate_mesh(box.mesh, &error) || !cad.validate_mesh(cylinder.mesh, &error)) {
        std::cerr << "CAD validation failed: " << error << "\n";
        return EXIT_FAILURE;
    }
    if (cad.assembly().parts.size() != 2 || box.mesh.triangles.size() != 12) return EXIT_FAILURE;

    gpu_engine::compute::Device device;
    device.name = "Blackwell-Inspired Reference GPU";
    device.compute_units = 128;
    device.lanes_per_wave = 32;
    gpu_engine::compute::ComputeRuntime runtime(device);
    std::vector<std::size_t> ids;
    runtime.dispatch({1000, 128}, [&](std::size_t id) { ids.push_back(id); });
    if (ids.size() != 1000 || ids.front() != 0 || ids.back() != 999) return EXIT_FAILURE;

    std::cout << "GPU CAD engine smoke test passed: " << cad.assembly().parts.size()
              << " parts, " << ids.size() << " compute elements\n";
    return EXIT_SUCCESS;
}
