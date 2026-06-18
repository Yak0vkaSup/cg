#pragma once
#include "gl_common.h"

class Framebuffer {
public:
    void create(int width, int height);
    void resize(int width, int height);
    void destroy();

    void bind() const;
    static void bindDefault(int width, int height);

    GLuint colorTexture() const { return colorTex_; }

private:
    GLuint fbo_ = 0;
    GLuint colorTex_ = 0;
    GLuint depthRbo_ = 0;
    int w_ = 0, h_ = 0;
};
