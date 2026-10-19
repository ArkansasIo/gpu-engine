#include "gpu_engine/cad.hpp"
#include "gpu_engine/cad_render.hpp"

#include <cassert>

int main() {
    gpu_engine::cad::CadEngine cad;
    auto& part = cad.add_box("RenderBox", {-1, -1, -1}, {1, 1, 1});
    const auto result = gpu_engine::cad_render::prepare_mesh(part.mesh);
    assert(result.status.ok);
    assert(result.data.vertex_count == part.mesh.vertices.size());
    assert(result.data.index_count == part.mesh.triangles.size() * 3);
    assert(result.data.vertex_buffer.size_bytes() > 0);
    assert(result.data.index_buffer.size_bytes() > 0);
    return 0;
}
