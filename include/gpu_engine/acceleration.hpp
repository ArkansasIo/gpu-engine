#pragma once

#include "gpu_engine/cad.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace gpu_engine::accel {

struct Ray { math::Vec3 origin; math::Vec3 direction; };
struct Hit { bool hit{false}; double t{0}; std::uint32_t triangle{0}; };

class Bvh {
public:
    void build(const cad::Mesh& mesh);
    Hit intersect(const cad::Mesh& mesh, const Ray& ray) const;
    std::size_t node_count() const noexcept { return nodes_.size(); }

private:
    struct Node {
        cad::Aabb bounds;
        std::uint32_t first{0};
        std::uint32_t count{0};
        std::int32_t left{-1};
        std::int32_t right{-1};
    };
    std::vector<std::uint32_t> triangle_indices_;
    std::vector<Node> nodes_;
};

} // namespace gpu_engine::accel
