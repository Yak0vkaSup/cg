#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

layout(std140, binding = 0) uniform Camera {
    mat4 uView;
    mat4 uProj;
    vec4 uCamPos;
    vec4 uOptions;
    vec4 uOptions2;
};

layout(std140, binding = 1) uniform Object {
    mat4 uModel;
    mat4 uNormalMat;
    vec4 uKa;
    vec4 uKd;
    vec4 uKs;
    vec4 uParams;
};

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;

void main() {
    vec4 world = uModel * vec4(aPos, 1.0);
    vWorldPos = world.xyz;
    vNormal   = mat3(uNormalMat) * aNormal;
    vUV       = aUV;
    gl_Position = uProj * uView * world;
}
