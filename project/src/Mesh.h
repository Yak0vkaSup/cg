#pragma once
#include "gl_common.h"
#include <vector>
#include <cstdint>

struct Mesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;
    GLsizei count = 0;

    GLuint instanceVBO = 0;
    GLsizei instanceCount = 0;

    void upload(const std::vector<Vertex>& vertices,
                const std::vector<uint32_t>& indices);
    void draw() const;

    void setInstanceMatrices(const std::vector<Mat4>& matrices);
    void drawInstanced() const;

    static Mesh makeCube();
    static Mesh makeSphere(int stacks = 24, int slices = 32);
    static Mesh makeFullscreenQuad();
};
