#pragma once
#include "mat4.h"

class Camera {
public:
    Vec3  target = {0, 0, 0};
    float radius = 6.0f;
    float phi    = 0.6f;
    float theta  = 0.4f;
    float fovYdeg = 60.0f;

    void orbit(float dPhi, float dTheta);
    void zoom(float dRadius);

    Vec3 position() const;
    Mat4 view() const;
    Mat4 proj(float aspect) const;
};
