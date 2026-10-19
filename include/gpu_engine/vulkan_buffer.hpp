#pragma once

#include "gpu_engine/buffer.hpp"
#include "gpu_engine/vulkan_device.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>

namespace gpu_engine::vulkan {

class Buffer {
public:
    Buffer();
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Status create(Device& device, const memory::BufferDesc& desc);
    void destroy() noexcept;
    bool is_initialized() const noexcept;
    std::size_t size_bytes() const noexcept;

    Status upload(const void* data, std::size_t size, std::size_t offset = 0);
    Status read(void* data, std::size_t size, std::size_t offset = 0);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace gpu_engine::vulkan
