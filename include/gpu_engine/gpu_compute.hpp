#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace gpu_engine::compute {

// Software model of a GPU execution hierarchy. It is intentionally backend-neutral:
// it can be replaced by CUDA/Vulkan/D3D12/Metal kernels without changing CAD code.
struct Device {
    std::string name{"Reference GPU"};
    std::uint32_t compute_units{1};
    std::uint32_t lanes_per_wave{32};
    std::size_t memory_bytes{0};
    bool hardware_accelerated{false};
};

struct Dispatch {
    std::size_t elements{0};
    std::size_t workgroup_size{256};
};

class ComputeRuntime {
public:
    explicit ComputeRuntime(Device device = {});
    const Device& device() const noexcept { return device_; }

    // Reference dispatch: executes a kernel over element IDs. The same contract
    // is suitable for a real GPU backend later.
    void dispatch(const Dispatch& dispatch, const std::function<void(std::size_t)>& kernel) const;

private:
    Device device_;
};

} // namespace gpu_engine::compute
