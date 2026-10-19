#version 450

layout(local_size_x = 256) in;

layout(std430, binding = 0) readonly buffer Positions {
    vec4 positions[];
};

layout(std430, binding = 1) writeonly buffer Transformed {
    vec4 transformed[];
};

layout(push_constant) uniform Transform {
    mat4 mvp;
} transform;

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index >= positions.length()) return;
    transformed[index] = transform.mvp * positions[index];
}
