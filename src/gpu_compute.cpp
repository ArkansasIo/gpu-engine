#include "gpu_engine/gpu_compute.hpp"

#include <algorithm>

namespace gpu_engine::compute {

ComputeRuntime::ComputeRuntime(Device device) : device_(std::move(device)) {}

void ComputeRuntime::dispatch(const Dispatch& d, const std::function<void(std::size_t)>& kernel) const {
    if (!kernel || d.elements==0) return;
    const std::size_t group=std::max<std::size_t>(1,d.workgroup_size);
    for (std::size_t base=0;base<d.elements;base+=group) {
        const std::size_t end=std::min(base+group,d.elements);
        for (std::size_t id=base;id<end;++id) kernel(id);
    }
}

} // namespace gpu_engine::compute
