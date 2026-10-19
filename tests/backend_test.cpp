#include "gpu_engine/backend.hpp"
#include "gpu_engine/gpu_engine.hpp"
#include "gpu_engine/vulkan_backend.hpp"

#include <cassert>
#include <iostream>

int main() {
    const auto info = gpu_engine::get_engine_info();
    assert(info.api_major == 0);
    assert(info.api_minor == 3);
    assert(std::string(gpu_engine::backend_name(gpu_engine::Backend::Vulkan)) == "Vulkan");

    auto backend = gpu_engine::backend::create_backend(gpu_engine::Backend::Vulkan);
    assert(backend);
    assert(backend->type() == gpu_engine::Backend::Vulkan);

    const auto status = backend->initialize();
    if (status.ok) {
        const auto devices = backend->devices();
        for (const auto& device : devices) {
            assert(device.capabilities.backend == gpu_engine::Backend::Vulkan);
            assert(!device.capabilities.name.empty());
            assert(device.capabilities.compute_supported || device.capabilities.graphics_supported);
        }
    } else {
        // A machine without the Vulkan SDK is a supported configuration.
        assert(status.message.find("Vulkan") != std::string::npos);
    }

    backend->shutdown();
    assert(!backend->is_initialized());

    const auto public_devices = gpu_engine::enumerate_devices();
    std::cout << "Detected GPU devices: " << public_devices.size() << '\n';
    return 0;
}
