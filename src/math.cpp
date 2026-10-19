#include "gpu_engine/math.hpp"

namespace gpu_engine::math {

Mat4 Mat4::identity() {
    Mat4 r{};
    for (int i=0;i<4;++i) r.m[i][i]=1.0;
    return r;
}

Mat4 Mat4::translation(const Vec3& v) {
    Mat4 r=identity();
    r.m[0][3]=v.x; r.m[1][3]=v.y; r.m[2][3]=v.z;
    return r;
}

Vec3 Mat4::transform_point(const Vec3& v) const {
    const double x=m[0][0]*v.x+m[0][1]*v.y+m[0][2]*v.z+m[0][3];
    const double y=m[1][0]*v.x+m[1][1]*v.y+m[1][2]*v.z+m[1][3];
    const double z=m[2][0]*v.x+m[2][1]*v.y+m[2][2]*v.z+m[2][3];
    const double w=m[3][0]*v.x+m[3][1]*v.y+m[3][2]*v.z+m[3][3];
    return w != 0.0 ? Vec3{x/w,y/w,z/w} : Vec3{x,y,z};
}

} // namespace gpu_engine::math
