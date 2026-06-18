#include "Camera.h"
#include <cmath>

static const float PI = 3.14159265358979f;

void Camera::orbit(float dPhi, float dTheta) {
    phi += dPhi;
    theta += dTheta;
    float lim = PI * 0.5f - 0.01f;
    if (theta > lim) theta = lim;
    if (theta < -lim) theta = -lim;
}

void Camera::zoom(float dRadius) {
    radius += dRadius;
    if (radius < 1.0f) radius = 1.0f;
    if (radius > 50.0f) radius = 50.0f;
}

Vec3 Camera::position() const {
    float x = radius * std::cos(theta) * std::cos(phi);
    float y = radius * std::sin(theta);
    float z = radius * std::cos(theta) * std::sin(phi);
    return { target.x + x, target.y + y, target.z + z };
}

Mat4 Camera::view() const {
    return mat4_lookAt(position(), target, {0, 1, 0});
}

Mat4 Camera::proj(float aspect) const {
    return mat4_perspective(fovYdeg * PI / 180.0f, aspect, 0.1f, 100.0f);
}
