#include "gpu_engine/cad.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace gpu_engine::cad {

void Aabb::expand(const Vec3& p) {
    if (!valid) { min=max=p; valid=true; return; }
    min.x=std::min(min.x,p.x); min.y=std::min(min.y,p.y); min.z=std::min(min.z,p.z);
    max.x=std::max(max.x,p.x); max.y=std::max(max.y,p.y); max.z=std::max(max.z,p.z);
}

void Aabb::expand(const Aabb& b) {
    if (!b.valid) return;
    expand(b.min); expand(b.max);
}

static void finish_bounds(Mesh& m) {
    m.bounds={};
    for (const auto& v : m.vertices) m.bounds.expand(v.position);
}

CadEngine::CadEngine() { assembly_.name="GPU CAD Assembly"; }

Part& CadEngine::add_box(const std::string& name, const Vec3& lo, const Vec3& hi) {
    Part p; p.name=name;
    const Vec3 v[8]={{lo.x,lo.y,lo.z},{hi.x,lo.y,lo.z},{hi.x,hi.y,lo.z},{lo.x,hi.y,lo.z},
                     {lo.x,lo.y,hi.z},{hi.x,lo.y,hi.z},{hi.x,hi.y,hi.z},{lo.x,hi.y,hi.z}};
    for (const auto& x:v) p.mesh.vertices.push_back({x});
    const std::uint32_t t[][3]={{0,1,2},{0,2,3},{4,6,5},{4,7,6},{0,4,5},{0,5,1},
                                {3,2,6},{3,6,7},{0,3,7},{0,7,4},{1,5,6},{1,6,2}};
    for (const auto& x:t) p.mesh.triangles.push_back({x[0],x[1],x[2]});
    finish_bounds(p.mesh);
    assembly_.parts.push_back(std::move(p));
    return assembly_.parts.back();
}

Part& CadEngine::add_cylinder(const std::string& name, double radius, double height, std::uint32_t segments) {
    if (segments < 3) segments=3;
    Part p; p.name=name;
    const double half=height*0.5;
    for (std::uint32_t i=0;i<segments;++i) {
        const double a=2.0*3.14159265358979323846*double(i)/double(segments);
        p.mesh.vertices.push_back({{radius*std::cos(a),radius*std::sin(a),-half}});
        p.mesh.vertices.push_back({{radius*std::cos(a),radius*std::sin(a), half}});
    }
    for (std::uint32_t i=0;i<segments;++i) {
        const std::uint32_t n=(i+1)%segments, b=2*i, t=b+1, nb=2*n, nt=nb+1;
        p.mesh.triangles.push_back({b,nb,nt}); p.mesh.triangles.push_back({b,nt,t});
    }
    finish_bounds(p.mesh);
    assembly_.parts.push_back(std::move(p));
    return assembly_.parts.back();
}

bool CadEngine::validate_mesh(const Mesh& mesh, std::string* error) const {
    for (std::size_t i=0;i<mesh.triangles.size();++i) {
        const auto& t=mesh.triangles[i];
        if (t.a>=mesh.vertices.size() || t.b>=mesh.vertices.size() || t.c>=mesh.vertices.size()) {
            if (error) *error="Triangle " + std::to_string(i) + " references a missing vertex";
            return false;
        }
        if (t.a==t.b || t.b==t.c || t.a==t.c) {
            if (error) *error="Triangle " + std::to_string(i) + " is degenerate";
            return false;
        }
    }
    return true;
}

} // namespace gpu_engine::cad
