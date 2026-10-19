#include "gpu_engine/vulkan_buffer.hpp"
#include "gpu_engine/vulkan_device.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
    gpu_engine::vulkan::Device device;
    const auto status = device.initialize();

    if (!status.ok) {
        // Vulkan is optional in CI. A missing SDK/driver is a valid skip condition.
        std::cout << "Vulkan unavailable: " << status.message << '\n';
        return 0;
    }

    assert(device.is_initialized());
    assert(device.info().capabilities.compute_supported);

    gpu_engine::vulkan::Buffer buffer;
    gpu_engine::memory::BufferDesc desc;
    desc.size_bytes = sizeof(std::uint32_t) * 4;
    desc.usage = gpu_engine::memory::BufferUsage::Storage |
                 gpu_engine::memory::BufferUsage::TransferSource |
                 gpu_engine::memory::BufferUsage::TransferDestination;
    desc.device_local = false;

    auto buffer_status = buffer.create(device, desc);
    assert(buffer_status.ok);
    assert(buffer.is_initialized());

    const std::array<std::uint32_t, 4> input{1, 2, 3, 4};
    std::array<std::uint32_t, 4> output{};
    assert(buffer.upload(input.data(), sizeof(input)).ok);
    assert(buffer.read(output.data(), sizeof(output)).ok);
    assert(output == input);
    assert(device.synchronize().ok);

    buffer.destroy();
    device.shutdown();
    return 0;
}
