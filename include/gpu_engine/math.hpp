#pragma once

#include <cmath>

namespace gpu_engine::math {

struct Vec3 {
    double x{0.0};
    double y{0.0};
    double z{0.0};

    constexpr Vec3 operator+(const Vec3& r) const { return {x+r.x,y+r.y,z+r.z}; }
    constexpr Vec3 operator-(const Vec3& r) const { return {x-r.x,y-r.y,z-r.z}; }
    constexpr Vec3 operator*(double s) const { return {x*s,y*s,z*s}; }
    constexpr Vec3 operator/(double s) const { return {x/s,y/s,z/s}; }
};

constexpr double dot(const Vec3& a, const Vec3& b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
constexpr Vec3 cross(const Vec3& a, const Vec3& b) {
    return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
}
inline double length(const Vec3& v) { return std::sqrt(dot(v,v)); }
inline Vec3 normalize(const Vec3& v) { const double n=length(v); return n > 1e-12 ? v/n : Vec3{}; }

struct Mat4 {
    double m[4][4]{};
    static Mat4 identity();
    static Mat4 translation(const Vec3& v);
    Vec3 transform_point(const Vec3& v) const;
};

} // namespace gpu_engine::math
