#pragma once

#include <cmath>

struct Vec2 {
  float x, y;
};

struct Vec3 {
  float x, y, z;
};

struct Vec4 {
  float x, y, z, w;
};

inline Vec3 vec3_add(Vec3 a, Vec3 b) {
  return {a.x + b.x, a.y + b.y, a.z + b.z};
}
inline Vec3 vec3_sub(Vec3 a, Vec3 b) {
  return {a.x - b.x, a.y - b.y, a.z - b.z};
}
inline Vec3 vec3_scale(Vec3 a, float s) { return {a.x * s, a.y * s, a.z * s}; }
inline float vec3_dot(Vec3 a, Vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline Vec3 vec3_cross(Vec3 a, Vec3 b) {
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
inline float vec3_len(Vec3 v) { return std::sqrt(vec3_dot(v, v)); }
inline Vec3 vec3_norm(Vec3 v) {
  float n = std::sqrt(vec3_dot(v, v));
  if (n < 1e-8f)
    return {0, 0, 0};
  return {v.x / n, v.y / n, v.z / n};
}

struct Mat4 {
  float m[16];
};

inline Mat4 mat4_identity() {
  Mat4 r{};
  r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
  return r;
}

inline Mat4 mat4_mul(const Mat4 &a, const Mat4 &b) {
  Mat4 r{};
  for (int c = 0; c < 4; c++) {
    for (int rIdx = 0; rIdx < 4; rIdx++) {
      float s = 0.0f;
      for (int k = 0; k < 4; k++) {
        s += a.m[k * 4 + rIdx] * b.m[c * 4 + k];
      }
      r.m[c * 4 + rIdx] = s;
    }
  }
  return r;
}

inline Vec4 mat4_mul_vec4(const Mat4 &a, Vec4 v) {
  return {
      a.m[0] * v.x + a.m[4] * v.y + a.m[8] * v.z + a.m[12] * v.w,
      a.m[1] * v.x + a.m[5] * v.y + a.m[9] * v.z + a.m[13] * v.w,
      a.m[2] * v.x + a.m[6] * v.y + a.m[10] * v.z + a.m[14] * v.w,
      a.m[3] * v.x + a.m[7] * v.y + a.m[11] * v.z + a.m[15] * v.w,
  };
}

inline Mat4 mat4_translation(float x, float y, float z) {
  Mat4 r = mat4_identity();
  r.m[12] = x;
  r.m[13] = y;
  r.m[14] = z;
  return r;
}

inline Mat4 mat4_scale(float x, float y, float z) {
  Mat4 r{};
  r.m[0] = x;
  r.m[5] = y;
  r.m[10] = z;
  r.m[15] = 1.0f;
  return r;
}

inline Mat4 mat4_rotX(float a) {
  Mat4 r = mat4_identity();
  float c = std::cos(a), s = std::sin(a);
  r.m[5] = c;
  r.m[6] = s;
  r.m[9] = -s;
  r.m[10] = c;
  return r;
}

inline Mat4 mat4_rotY(float a) {
  Mat4 r = mat4_identity();
  float c = std::cos(a), s = std::sin(a);
  r.m[0] = c;
  r.m[2] = -s;
  r.m[8] = s;
  r.m[10] = c;
  return r;
}

inline Mat4 mat4_rotZ(float a) {
  Mat4 r = mat4_identity();
  float c = std::cos(a), s = std::sin(a);
  r.m[0] = c;
  r.m[1] = s;
  r.m[4] = -s;
  r.m[5] = c;
  return r;
}

inline Mat4 mat4_lookAt(Vec3 position, Vec3 target, Vec3 up) {
  Vec3 f = vec3_norm(vec3_sub(position, target));
  Vec3 r = vec3_norm(vec3_cross(up, f));
  Vec3 u = vec3_cross(f, r);

  Mat4 v{};
  v.m[0] = r.x;
  v.m[1] = u.x;
  v.m[2] = f.x;
  v.m[3] = 0;
  v.m[4] = r.y;
  v.m[5] = u.y;
  v.m[6] = f.y;
  v.m[7] = 0;
  v.m[8] = r.z;
  v.m[9] = u.z;
  v.m[10] = f.z;
  v.m[11] = 0;
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

inline Mat4 mat4_normal_matrix(const Mat4 &w) {

  float a = w.m[0], b = w.m[4], c = w.m[8];
  float d = w.m[1], e = w.m[5], f = w.m[9];
  float g = w.m[2], h = w.m[6], i = w.m[10];
  float A = (e * i - f * h);
  float B = -(d * i - f * g);
  float C = (d * h - e * g);
  float det = a * A + b * B + c * C;
  Mat4 r = mat4_identity();
  if (std::fabs(det) < 1e-8f)
    return r;
  float invDet = 1.0f / det;

  float D = -(b * i - c * h), E = (a * i - c * g), F = -(a * h - b * g);
  float G = (b * f - c * e), H = -(a * f - c * d), I = (a * e - b * d);
  float inv[9] = {A * invDet, B * invDet, C * invDet, D * invDet, E * invDet,
                  F * invDet, G * invDet, H * invDet, I * invDet};

  r.m[0] = inv[0];
  r.m[1] = inv[3];
  r.m[2] = inv[6];
  r.m[4] = inv[1];
  r.m[5] = inv[4];
  r.m[6] = inv[7];
  r.m[8] = inv[2];
  r.m[9] = inv[5];
  r.m[10] = inv[8];
  return r;
}

static const float PI = 3.14159265358979f;
