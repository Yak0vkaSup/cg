#pragma once
#include "gl_common.h"

struct Material {
    Vec3  ambient   = {0.1f, 0.1f, 0.1f};
    Vec3  diffuse   = {0.8f, 0.8f, 0.8f};
    Vec3  specular  = {0.5f, 0.5f, 0.5f};
    float shininess = 32.0f;

    GLuint diffuseTex = 0;
    bool   hasDiffuseTex = false;
};
