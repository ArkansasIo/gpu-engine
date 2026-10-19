#pragma once

#include "gpu_engine/backend.hpp"

#include <memory>

namespace gpu_engine::vulkan {

class Backend final : public backend::IBackend {
public:
    Backend();
    ~Backend() override;

    Backend(const Backend&) = delete;
    Backend& operator=(const Backend&) = delete;

    gpu_engine::Backend type() const noexcept override;
    Status initialize() override;
    void shutdown() noexcept override;
    bool is_initialized() const noexcept override;
    std::vector<backend::DeviceInfo> devices() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace gpu_engine::vulkan
