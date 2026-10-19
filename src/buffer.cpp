#include "gpu_engine/buffer.hpp"

#include <cstring>

namespace gpu_engine::memory {

Buffer::Buffer(const BufferDesc& desc) {
    resize(desc);
}

Status Buffer::resize(const BufferDesc& desc) {
    if (desc.size_bytes > (static_cast<std::size_t>(-1) / 2)) {
        return Status::failure("Buffer size is unreasonably large");
    }
    desc_ = desc;
    storage_.assign(desc_.size_bytes, 0);
    return Status::success();
}

void Buffer::reset() noexcept {
    storage_.clear();
    storage_.shrink_to_fit();
    desc_ = {};
}

Status Buffer::upload(const void* data, std::size_t size, std::size_t offset) {
    if (size == 0) return Status::success();
    if (data == nullptr) return Status::failure("Buffer upload source is null");
    if (offset > desc_.size_bytes || size > desc_.size_bytes - offset) {
        return Status::failure("Buffer upload range exceeds buffer size");
    }
    std::memcpy(storage_.data() + offset, data, size);
    return Status::success();
}

Status Buffer::read(void* data, std::size_t size, std::size_t offset) const {
    if (size == 0) return Status::success();
    if (data == nullptr) return Status::failure("Buffer read destination is null");
    if (offset > desc_.size_bytes || size > desc_.size_bytes - offset) {
        return Status::failure("Buffer read range exceeds buffer size");
    }
    std::memcpy(data, storage_.data() + offset, size);
    return Status::success();
}

} // namespace gpu_engine::memory
