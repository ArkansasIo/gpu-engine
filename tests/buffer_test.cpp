#include "gpu_engine/buffer.hpp"

#include <array>
#include <cassert>

int main() {
    using namespace gpu_engine::memory;
    Buffer buffer(BufferDesc{sizeof(std::array<std::uint32_t, 4>),
                             BufferUsage::Storage | BufferUsage::TransferDestination,
                             false});
    assert(buffer.valid());

    const std::array<std::uint32_t, 4> source{1, 2, 3, 4};
    std::array<std::uint32_t, 4> destination{};
    assert(buffer.upload(source.data(), sizeof(source)).ok);
    assert(buffer.read(destination.data(), sizeof(destination)).ok);
    assert(destination == source);

    assert(!buffer.upload(source.data(), sizeof(source), 2).ok);
    assert(!buffer.read(nullptr, 1).ok);
    buffer.reset();
    assert(buffer.size_bytes() == 0);
    return 0;
}
