#pragma once

#include "gpu_engine/gpu_engine.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace gpu_engine::backend {

struct QueueInfo {
    std::uint32_t family_index{0};
    bool graphics{false};
    bool compute{false};
    bool present{false};
};

struct DeviceInfo {
    DeviceCapabilities capabilities;
    std::vector<QueueInfo> queues;
};

class IBackend {
public:
    virtual ~IBackend() = default;
    virtual Backend type() const noexcept = 0;
    virtual Status initialize() = 0;
    virtual void shutdown() noexcept = 0;
    virtual bool is_initialized() const noexcept = 0;
    virtual std::vector<DeviceInfo> devices() const = 0;
};

using BackendPtr = std::unique_ptr<IBackend>;

BackendPtr create_backend(Backend backend);
std::vector<DeviceInfo> enumerate_backend_devices();

} // namespace gpu_engine::backend
