#pragma once

#include "gpu_engine/gpu_engine.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace gpu_engine::memory {

enum class BufferUsage : std::uint32_t {
    None = 0,
    Vertex = 1u << 0,
    Index = 1u << 1,
    Uniform = 1u << 2,
    Storage = 1u << 3,
    TransferSource = 1u << 4,
    TransferDestination = 1u << 5
};

constexpr BufferUsage operator|(BufferUsage a, BufferUsage b) noexcept {
    return static_cast<BufferUsage>(static_cast<std::uint32_t>(a) |
                                    static_cast<std::uint32_t>(b));
}

struct BufferDesc {
    std::size_t size_bytes{0};
    BufferUsage usage{BufferUsage::None};
    bool device_local{false};
};

class Buffer {
public:
    Buffer() = default;
    explicit Buffer(const BufferDesc& desc);

    Status resize(const BufferDesc& desc);
    void reset() noexcept;

    bool valid() const noexcept { return !storage_.empty() || desc_.size_bytes == 0; }
    std::size_t size_bytes() const noexcept { return desc_.size_bytes; }
    const BufferDesc& descriptor() const noexcept { return desc_; }

    Status upload(const void* data, std::size_t size, std::size_t offset = 0);
    Status read(void* data, std::size_t size, std::size_t offset = 0) const;

private:
    BufferDesc desc_;
    std::vector<std::uint8_t> storage_;
};

} // namespace gpu_engine::memory
