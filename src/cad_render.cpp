#include "gpu_engine/cad_render.hpp"

#include <cstdint>
#include <vector>

namespace gpu_engine::cad_render {

UploadResult prepare_mesh(const cad::Mesh& mesh) {
    UploadResult result;
    result.status = Status::success();

    std::vector<math::Vec3> positions;
    positions.reserve(mesh.vertices.size());
    for (const auto& vertex : mesh.vertices) positions.push_back(vertex.position);

    std::vector<std::uint32_t> indices;
    indices.reserve(mesh.triangles.size() * 3);
    for (const auto& triangle : mesh.triangles) {
        if (triangle.a >= mesh.vertices.size() ||
            triangle.b >= mesh.vertices.size() ||
            triangle.c >= mesh.vertices.size()) {
            result.status = Status::failure("CAD mesh contains an invalid triangle index");
            return result;
        }
        indices.push_back(triangle.a);
        indices.push_back(triangle.b);
        indices.push_back(triangle.c);
    }

    result.data.vertex_count = positions.size();
    result.data.index_count = indices.size();
    result.data.vertex_buffer.resize(memory::BufferDesc{
        positions.size() * sizeof(math::Vec3), memory::BufferUsage::Vertex, true});
    result.data.index_buffer.resize(memory::BufferDesc{
        indices.size() * sizeof(std::uint32_t), memory::BufferUsage::Index, true});

    if (!positions.empty()) {
        result.status = result.data.vertex_buffer.upload(
            positions.data(), positions.size() * sizeof(math::Vec3));
        if (!result.status.ok) return result;
    }
    if (!indices.empty()) {
        result.status = result.data.index_buffer.upload(
            indices.data(), indices.size() * sizeof(std::uint32_t));
    }
    return result;
}

} // namespace gpu_engine::cad_render
