#pragma once

#include <glad/gl.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "math3d.h"

namespace glu {

inline std::string read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        std::fprintf(stderr, "[shader] cannot open %s\n", path.c_str());
        return {};
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

inline GLuint compile(GLenum type, const std::string& src, const char* tag) {
    GLuint s = glCreateShader(type);
    const char* p = src.c_str();
    glShaderSource(s, 1, &p, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[shader compile error] %s:\n%s\n", tag, log);
    }
    return s;
}

inline bool link_check(GLuint p, const char* tag) {
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[program link error] %s:\n%s\n", tag, log);
        return false;
    }
    return true;
}

inline GLuint program_vf(const std::string& vsPath, const std::string& fsPath) {
    GLuint vs = compile(GL_VERTEX_SHADER,   read_file(vsPath), vsPath.c_str());
    GLuint fs = compile(GL_FRAGMENT_SHADER, read_file(fsPath), fsPath.c_str());
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    link_check(p, vsPath.c_str());
    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}

struct Framebuffer {
    GLuint fbo = 0, color = 0, depth = 0;
    int w = 0, h = 0;

    void create(int width, int height) {
        destroy();
        w = width; h = height;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &color);
        glBindTexture(GL_TEXTURE_2D, color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

        glGenRenderbuffers(1, &depth);
        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::fprintf(stderr, "[fbo] incomplete framebuffer\n");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void resize(int width, int height) {
        if (width == w && height == h) return;
        create(width, height);
    }

    void destroy() {
        if (color) glDeleteTextures(1, &color);
        if (depth) glDeleteRenderbuffers(1, &depth);
        if (fbo)   glDeleteFramebuffers(1, &fbo);
        fbo = color = depth = 0;
    }
};

GLuint load_texture_2d(const std::string& path, bool srgb);

void save_screenshot(const std::string& path, int w, int h);

inline GLuint white_texture() {
    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    unsigned char px[4] = { 255, 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return t;
}

inline GLuint make_sky_cubemap(int size = 256) {
    GLuint cube;
    glGenTextures(1, &cube);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube);

    auto sky = [](Vec3 d) -> Vec3 {
        d = vec3_norm(d);
        float t = 0.5f * (d.y + 1.0f);
        Vec3 horizon { 0.80f, 0.86f, 0.92f };
        Vec3 zenith  { 0.18f, 0.32f, 0.62f };
        Vec3 ground  { 0.22f, 0.20f, 0.18f };
        Vec3 base;
        if (d.y >= 0.0f) {
            float k = std::pow(t, 0.6f);
            base = vec3_add(vec3_scale(horizon, 1.0f - k), vec3_scale(zenith, k));
        } else {
            float k = std::pow(-d.y, 0.5f);
            base = vec3_add(vec3_scale(horizon, 1.0f - k), vec3_scale(ground, k));
        }

        Vec3 sunDir = vec3_norm(Vec3{ 0.5f, 0.8f, 0.4f });
        float s = vec3_dot(d, sunDir);
        if (s > 0.0f) {
            float glow = std::pow(s, 350.0f) * 3.0f + std::pow(s, 8.0f) * 0.15f;
            base = vec3_add(base, vec3_scale(Vec3{ 1.0f, 0.95f, 0.8f }, glow));
        }
        return base;
    };

    struct Face { GLenum target; Vec3 forward, right, up; };
    Face faces[6] = {
        { GL_TEXTURE_CUBE_MAP_POSITIVE_X, { 1,0,0},  {0,0,-1}, {0,-1,0} },
        { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, {-1,0,0},  {0,0, 1}, {0,-1,0} },
        { GL_TEXTURE_CUBE_MAP_POSITIVE_Y, { 0,1,0},  {1,0, 0}, {0, 0,1} },
        { GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, { 0,-1,0}, {1,0, 0}, {0, 0,-1} },
        { GL_TEXTURE_CUBE_MAP_POSITIVE_Z, { 0,0,1},  {1,0, 0}, {0,-1,0} },
        { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, { 0,0,-1}, {-1,0,0}, {0,-1,0} },
    };

    std::vector<float> buf(size * size * 3);
    for (const Face& fc : faces) {
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                float u = (2.0f * (x + 0.5f) / size) - 1.0f;
                float v = (2.0f * (y + 0.5f) / size) - 1.0f;
                Vec3 dir = vec3_add(fc.forward,
                            vec3_add(vec3_scale(fc.right, u), vec3_scale(fc.up, v)));
                Vec3 c = sky(dir);
                int i = (y * size + x) * 3;
                buf[i + 0] = c.x; buf[i + 1] = c.y; buf[i + 2] = c.z;
            }
        }
        glTexImage2D(fc.target, 0, GL_RGB16F, size, size, 0, GL_RGB, GL_FLOAT, buf.data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    return cube;
}

}
