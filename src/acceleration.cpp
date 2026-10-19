#include "gpu_engine/acceleration.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>

namespace gpu_engine::accel {
namespace {

constexpr double kEpsilon = 1e-12;

cad::Aabb triangle_bounds(const cad::Mesh& mesh, const cad::Triangle& tri) {
    cad::Aabb box;
    box.expand(mesh.vertices[tri.a].position);
    box.expand(mesh.vertices[tri.b].position);
    box.expand(mesh.vertices[tri.c].position);
    return box;
}

math::Vec3 centroid(const cad::Mesh& mesh, const cad::Triangle& tri) {
    const auto& a = mesh.vertices[tri.a].position;
    const auto& b = mesh.vertices[tri.b].position;
    const auto& c = mesh.vertices[tri.c].position;
    return (a + b + c) / 3.0;
}

bool ray_aabb(const Ray& ray, const cad::Aabb& box, double t_max) {
    if (!box.valid) return false;
    double t_min = 0.0;
    for (int axis = 0; axis < 3; ++axis) {
        const double origin = axis == 0 ? ray.origin.x : (axis == 1 ? ray.origin.y : ray.origin.z);
        const double direction = axis == 0 ? ray.direction.x : (axis == 1 ? ray.direction.y : ray.direction.z);
        const double lo = axis == 0 ? box.min.x : (axis == 1 ? box.min.y : box.min.z);
        const double hi = axis == 0 ? box.max.x : (axis == 1 ? box.max.y : box.max.z);
        if (std::abs(direction) <= kEpsilon) {
            if (origin < lo || origin > hi) return false;
            continue;
        }
        double a = (lo - origin) / direction;
        double b = (hi - origin) / direction;
        if (a > b) std::swap(a, b);
        t_min = std::max(t_min, a);
        t_max = std::min(t_max, b);
        if (t_max < t_min) return false;
    }
    return true;
}

bool ray_triangle(const Ray& ray, const math::Vec3& v0, const math::Vec3& v1,
                  const math::Vec3& v2, double& t) {
    // Möller–Trumbore intersection, accepting either triangle winding.
    const auto edge1 = v1 - v0;
    const auto edge2 = v2 - v0;
    const auto p = math::cross(ray.direction, edge2);
    const double determinant = math::dot(edge1, p);
    if (std::abs(determinant) <= kEpsilon) return false;
    const double inverse = 1.0 / determinant;
    const auto offset = ray.origin - v0;
    const double u = math::dot(offset, p) * inverse;
    if (u < 0.0 || u > 1.0) return false;
    const auto q = math::cross(offset, edge1);
    const double v = math::dot(ray.direction, q) * inverse;
    if (v < 0.0 || u + v > 1.0) return false;
    const double distance = math::dot(edge2, q) * inverse;
    if (distance < 0.0) return false;
    t = distance;
    return true;
}

} // namespace

void Bvh::build(const cad::Mesh& mesh) {
    triangle_indices_.clear();
    nodes_.clear();
    triangle_indices_.reserve(mesh.triangles.size());

    for (std::size_t i = 0; i < mesh.triangles.size(); ++i) {
        const auto& tri = mesh.triangles[i];
        if (tri.a >= mesh.vertices.size() || tri.b >= mesh.vertices.size() || tri.c >= mesh.vertices.size()) {
            // Invalid primitives are excluded rather than dereferenced.
            continue;
        }
        triangle_indices_.push_back(static_cast<std::uint32_t>(i));
    }
    if (!triangle_indices_.empty()) build_node(mesh, 0, triangle_indices_.size());
}

std::uint32_t Bvh::build_node(const cad::Mesh& mesh, std::size_t first, std::size_t count) {
    const auto node_index = static_cast<std::uint32_t>(nodes_.size());
    nodes_.push_back({});
    cad::Aabb bounds;
    cad::Aabb centroid_bounds;
    for (std::size_t i = first; i < first + count; ++i) {
        const auto& tri = mesh.triangles[triangle_indices_[i]];
        bounds.expand(triangle_bounds(mesh, tri));
        centroid_bounds.expand(centroid(mesh, tri));
    }
    nodes_[node_index].bounds = bounds;

    constexpr std::size_t kLeafTriangleCount = 4;
    if (count <= kLeafTriangleCount || !centroid_bounds.valid) {
        nodes_[node_index].first = static_cast<std::uint32_t>(first);
        nodes_[node_index].count = static_cast<std::uint32_t>(count);
        return node_index;
    }

    const auto extent = centroid_bounds.max - centroid_bounds.min;
    int axis = 0;
    if (extent.y > extent.x && extent.y >= extent.z) axis = 1;
    else if (extent.z > extent.x && extent.z > extent.y) axis = 2;
    const auto coordinate = [axis](const math::Vec3& p) {
        return axis == 0 ? p.x : (axis == 1 ? p.y : p.z);
    };
    const auto middle = first + count / 2;
    std::nth_element(triangle_indices_.begin() + static_cast<std::ptrdiff_t>(first),
                     triangle_indices_.begin() + static_cast<std::ptrdiff_t>(middle),
                     triangle_indices_.begin() + static_cast<std::ptrdiff_t>(first + count),
                     [&](std::uint32_t lhs, std::uint32_t rhs) {
                         return coordinate(centroid(mesh, mesh.triangles[lhs])) <
                                coordinate(centroid(mesh, mesh.triangles[rhs]));
                     });

    const auto left = build_node(mesh, first, middle - first);
    const auto right = build_node(mesh, middle, first + count - middle);
    nodes_[node_index].left = static_cast<std::int32_t>(left);
    nodes_[node_index].right = static_cast<std::int32_t>(right);
    return node_index;
}

Hit Bvh::intersect(const cad::Mesh& mesh, const Ray& ray) const {
    Hit closest;
    closest.t = std::numeric_limits<double>::infinity();
    if (nodes_.empty() || math::length(ray.direction) <= kEpsilon) {
        closest.t = 0.0;
        return closest;
    }

    std::vector<std::uint32_t> stack;
    stack.reserve(64);
    stack.push_back(0);
    while (!stack.empty()) {
        const auto node_index = stack.back();
        stack.pop_back();
        if (node_index >= nodes_.size()) continue;
        const auto& node = nodes_[node_index];
        if (!ray_aabb(ray, node.bounds, closest.t)) continue;
        if (node.count != 0) {
            const std::size_t end = std::min<std::size_t>(
                static_cast<std::size_t>(node.first) + node.count, triangle_indices_.size());
            for (std::size_t i = node.first; i < end; ++i) {
                const auto triangle_index = triangle_indices_[i];
                if (triangle_index >= mesh.triangles.size()) continue;
                const auto& tri = mesh.triangles[triangle_index];
                if (tri.a >= mesh.vertices.size() || tri.b >= mesh.vertices.size() || tri.c >= mesh.vertices.size()) continue;
                double t = 0.0;
                if (ray_triangle(ray, mesh.vertices[tri.a].position, mesh.vertices[tri.b].position,
                                 mesh.vertices[tri.c].position, t) && t < closest.t) {
                    closest.hit = true;
                    closest.t = t;
                    closest.triangle = triangle_index;
                }
            }
        } else {
            if (node.left >= 0) stack.push_back(static_cast<std::uint32_t>(node.left));
            if (node.right >= 0) stack.push_back(static_cast<std::uint32_t>(node.right));
        }
    }
    if (!closest.hit) closest.t = 0.0;
    return closest;
}

} // namespace gpu_engine::accel
