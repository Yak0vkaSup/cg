#pragma once
#include <cmath>

struct Vec3 {
    float x, y, z;
};

inline Vec3 vec3_sub(Vec3 a, Vec3 b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline float vec3_dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}
inline Vec3 vec3_norm(Vec3 v) {
    float n = std::sqrt(vec3_dot(v, v));
    if (n < 1e-8f) return { 0,0,0 };
    return { v.x/n, v.y/n, v.z/n };
}

struct Mat4 {
    float m[16];
};

inline Mat4 mat4_identity() {
    Mat4 r{};
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
    return r;
}

// Ex 1.1 multiplication de deux matrices
inline Mat4 mat4_mul(const Mat4& a, const Mat4& b) {
    Mat4 r{};
    for (int c = 0; c < 4; c++) {
        for (int rIdx = 0; rIdx < 4; rIdx++) {
            float s = 0.0f;
            for (int k = 0; k < 4; k++) {
                s += a.m[k*4 + rIdx] * b.m[c*4 + k];
            }
            r.m[c*4 + rIdx] = s;
        }
    }
    return r;
}

inline Mat4 mat4_translation(float x, float y, float z) {
    Mat4 r = mat4_identity();
    r.m[12] = x; r.m[13] = y; r.m[14] = z;
    return r;
}

inline Mat4 mat4_scale(float x, float y, float z) {
    Mat4 r{};
    r.m[0] = x; r.m[5] = y; r.m[10] = z; r.m[15] = 1.0f;
    return r;
}

inline Mat4 mat4_rotX(float a) {
    Mat4 r = mat4_identity();
    float c = std::cos(a), s = std::sin(a);
    r.m[5] = c;  r.m[6] = s;
    r.m[9] = -s; r.m[10] = c;
    return r;
}

inline Mat4 mat4_rotY(float a) {
    Mat4 r = mat4_identity();
    float c = std::cos(a), s = std::sin(a);
    r.m[0] = c;  r.m[2] = -s;
    r.m[8] = s;  r.m[10] = c;
    return r;
}

inline Mat4 mat4_rotZ(float a) {
    Mat4 r = mat4_identity();
    float c = std::cos(a), s = std::sin(a);
    r.m[0] = c;  r.m[1] = s;
    r.m[4] = -s; r.m[5] = c;
    return r;
}

// Ex 2.1 LookAt
inline Mat4 mat4_lookAt(Vec3 position, Vec3 target, Vec3 up) {
    Vec3 f = vec3_norm(vec3_sub(position, target));   // -(target-position)
    Vec3 r = vec3_norm(vec3_cross(up, f));
    Vec3 u = vec3_cross(f, r);

    Mat4 v{};
    v.m[0] = r.x; v.m[1] = u.x; v.m[2] = f.x; v.m[3] = 0;
    v.m[4] = r.y; v.m[5] = u.y; v.m[6] = f.y; v.m[7] = 0;
    v.m[8] = r.z; v.m[9] = u.z; v.m[10]= f.z; v.m[11]= 0;
    v.m[12] = -vec3_dot(r, position);
    v.m[13] = -vec3_dot(u, position);
    v.m[14] = -vec3_dot(f, position);
    v.m[15] = 1.0f;
    return v;
}

inline Mat4 mat4_perspective(float fovy, float aspect, float zn, float zf) {
    Mat4 r{};
    float f = 1.0f / std::tan(fovy * 0.5f);
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (zf + zn) / (zn - zf);
    r.m[11] = -1.0f;
    r.m[14] = (2.0f * zf * zn) / (zn - zf);
    return r;
}
