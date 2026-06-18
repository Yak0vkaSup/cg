#pragma once
#include "gl_common.h"

class CameraUBO {
public:
    static const GLuint BINDING = 0;

    void create();
    void destroy();
    void update(const Mat4& view, const Mat4& proj, Vec3 camPos);

private:
    GLuint ubo_ = 0;
};
