#pragma once

#include "gpu_engine/buffer.hpp"
#include "gpu_engine/cad.hpp"

#include <cstddef>

namespace gpu_engine::cad_render {

struct MeshGpuData {
    memory::Buffer vertex_buffer;
    memory::Buffer index_buffer;
    std::size_t vertex_count{0};
    std::size_t index_count{0};
};

struct UploadResult {
    Status status;
    MeshGpuData data;
};

UploadResult prepare_mesh(const cad::Mesh& mesh);

} // namespace gpu_engine::cad_render
