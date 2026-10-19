#include "gpu_engine/gpu_engine.hpp"

#include <utility>

namespace gpu_engine {

Status Status::success() { return Status{true, {}}; }

Status Status::failure(std::string message) {
    return Status{false, std::move(message)};
}

EngineInfo get_engine_info() { return EngineInfo{}; }

std::vector<DeviceCapabilities> enumerate_devices() {
    // Hardware discovery belongs to backend adapters, not the base API.
    return {};
}

const char* backend_name(Backend backend) noexcept {
    switch (backend) {
    case Backend::None: return "None";
    case Backend::Vulkan: return "Vulkan";
    case Backend::Direct3D12: return "Direct3D 12";
    case Backend::Metal: return "Metal";
    case Backend::WebGPU: return "WebGPU";
    }
    return "Unknown";
}

} // namespace gpu_engine
