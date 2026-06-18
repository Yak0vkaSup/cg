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

// programme compute (OpenGL 4.3)
inline GLuint program_compute(const std::string& csPath) {
    GLuint cs = compile(GL_COMPUTE_SHADER, read_file(csPath), csPath.c_str());
    GLuint p = glCreateProgram();
    glAttachShader(p, cs);
    glLinkProgram(p);
    link_check(p, csPath.c_str());
    glDeleteShader(cs);
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
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

//  cubemap depuis 6 images les png dans les asstes 
GLuint load_cubemap(const std::string& dir, const std::string& ext, bool srgb);

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

}
