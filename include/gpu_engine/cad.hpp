#pragma once

#include "gpu_engine/math.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace gpu_engine::cad {

using math::Vec3;
using math::Mat4;

struct Vertex { Vec3 position; };
struct Triangle { std::uint32_t a{}, b{}, c{}; };

struct Aabb {
    Vec3 min{0,0,0};
    Vec3 max{0,0,0};
    bool valid{false};
    void expand(const Vec3& p);
    void expand(const Aabb& b);
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
    Aabb bounds;
};

struct Part {
    std::string name;
    Mesh mesh;
    Mat4 transform{Mat4::identity()};
};

struct Assembly {
    std::string name{"Untitled Assembly"};
    std::vector<Part> parts;
};

class CadEngine {
public:
    CadEngine();

    Part& add_box(const std::string& name, const Vec3& min, const Vec3& max);
    Part& add_cylinder(const std::string& name, double radius, double height, std::uint32_t segments=32);
    const Assembly& assembly() const noexcept { return assembly_; }
    Assembly& assembly() noexcept { return assembly_; }

    // Deterministic tessellation validation for GPU upload preparation.
    bool validate_mesh(const Mesh& mesh, std::string* error=nullptr) const;

private:
    Assembly assembly_;
};

} // namespace gpu_engine::cad
