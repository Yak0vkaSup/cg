#pragma once

#include "math3d.h"

struct OrbitCamera {
    Vec3  target { 0.0f, 0.5f, 0.0f };
    float radius = 8.0f;
    float phi    = 0.9f;
    float theta  = 0.35f;

    float minRadius = 1.5f;
    float maxRadius = 40.0f;

    Vec3 position() const {
        float x = radius * std::cos(theta) * std::cos(phi);
        float y = radius * std::sin(theta);
        float z = radius * std::cos(theta) * std::sin(phi);
        return { target.x + x, target.y + y, target.z + z };
    }

    Mat4 view() const {
        return mat4_lookAt(position(), target, { 0, 1, 0 });
    }

    void rotate(float dPhi, float dTheta) {
        phi   += dPhi;
        theta += dTheta;
        if (phi >  PI) phi -= 2.0f * PI;
        if (phi < -PI) phi += 2.0f * PI;
        const float lim = PI * 0.5f - 0.01f;
        if (theta >  lim) theta =  lim;
        if (theta < -lim) theta = -lim;
    }

    void zoom(float delta) {
        radius -= delta;
        if (radius < minRadius) radius = minRadius;
        if (radius > maxRadius) radius = maxRadius;
    }
};
