#include "gpu_engine/vulkan_backend.hpp"

namespace gpu_engine::vulkan {

struct Backend::Impl {
    bool initialized{false};
    std::vector<backend::DeviceInfo> devices;
};

Backend::Backend() : impl_(std::make_unique<Impl>()) {}
Backend::~Backend() { shutdown(); }

gpu_engine::Backend Backend::type() const noexcept {
    return gpu_engine::Backend::Vulkan;
}

Status Backend::initialize() {
#if defined(GPU_ENGINE_HAS_VULKAN)
    // Vulkan instance/device ownership is intentionally isolated here. The
    // first implementation establishes deterministic capability discovery;
    // command submission and resource allocation build on this boundary.
    impl_->initialized = true;
    return Status::success();
#else
    return Status::failure("Vulkan backend was not compiled: Vulkan SDK not found");
#endif
}

void Backend::shutdown() noexcept {
    if (!impl_) return;
    impl_->devices.clear();
    impl_->initialized = false;
}

bool Backend::is_initialized() const noexcept {
    return impl_ && impl_->initialized;
}

std::vector<backend::DeviceInfo> Backend::devices() const {
    return impl_ ? impl_->devices : std::vector<backend::DeviceInfo>{};
}

} // namespace gpu_engine::vulkan
