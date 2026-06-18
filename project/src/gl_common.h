#pragma once
#include <glad/gl.h>
#include <cstddef>
#include "mat4.h"

struct Vertex {
    float px, py, pz;
    float nx, ny, nz;
    float u, v;
};
