#include "CameraUBO.h"

// Doit correspondre exactement au bloc std140 cote GLSL (voir CameraUBO.h).
struct CameraBlock {
    float view[16];   // offset 0
    float proj[16];   // offset 64
    float camPos[4];  // offset 128 (vec4 : xyz utilises, w ignore)
};

void CameraUBO::create() {
    glGenBuffers(1, &ubo_);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraBlock), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // attache le buffer au point de binding partage.
    glBindBufferBase(GL_UNIFORM_BUFFER, BINDING, ubo_);
}

void CameraUBO::destroy() {
    if (ubo_) glDeleteBuffers(1, &ubo_);
    ubo_ = 0;
}

void CameraUBO::update(const Mat4& view, const Mat4& proj, Vec3 camPos) {
    CameraBlock b{};
    for (int i = 0; i < 16; ++i) { b.view[i] = view.m[i]; b.proj[i] = proj.m[i]; }
    b.camPos[0] = camPos.x; b.camPos[1] = camPos.y; b.camPos[2] = camPos.z; b.camPos[3] = 1.0f;

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraBlock), &b);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
