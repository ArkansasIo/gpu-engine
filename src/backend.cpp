#include "gpu_engine/backend.hpp"
#include "gpu_engine/vulkan_backend.hpp"

namespace gpu_engine::backend {

BackendPtr create_backend(Backend backend) {
    switch (backend) {
    case Backend::Vulkan:
        return std::make_unique<vulkan::Backend>();
    case Backend::None:
    case Backend::Direct3D12:
    case Backend::Metal:
    case Backend::WebGPU:
        return nullptr;
    }
    return nullptr;
}

std::vector<DeviceInfo> enumerate_backend_devices() {
    std::vector<DeviceInfo> result;
    auto vulkan = create_backend(Backend::Vulkan);
    if (vulkan && vulkan->initialize().ok) {
        const auto devices = vulkan->devices();
        result.insert(result.end(), devices.begin(), devices.end());
    }
    return result;
}

} // namespace gpu_engine::backend
