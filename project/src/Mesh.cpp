#include "Mesh.h"
#include <cmath>

void Mesh::upload(const std::vector<Vertex>& vertices,
                  const std::vector<uint32_t>& indices) {
    count = (GLsizei)indices.size();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
                 indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, px));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::draw() const {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Mesh::setInstanceMatrices(const std::vector<Mat4>& matrices) {
    instanceCount = (GLsizei)matrices.size();

    glBindVertexArray(vao);

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, matrices.size() * sizeof(Mat4),
                 matrices.data(), GL_STATIC_DRAW);

    for (int col = 0; col < 4; ++col) {
        int location = 3 + col;
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE,
                              sizeof(Mat4), (void*)(col * 4 * sizeof(float)));
        glVertexAttribDivisor(location, 1);
    }

    glBindVertexArray(0);
}

void Mesh::drawInstanced() const {
    glBindVertexArray(vao);
    glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr, instanceCount);
    glBindVertexArray(0);
}

static void addFace(std::vector<Vertex>& v, std::vector<uint32_t>& idx,
                    Vec3 n, Vec3 a, Vec3 b, Vec3 c, Vec3 d) {
    uint32_t base = (uint32_t)v.size();
    v.push_back({a.x, a.y, a.z, n.x, n.y, n.z, 0.0f, 0.0f});
    v.push_back({b.x, b.y, b.z, n.x, n.y, n.z, 1.0f, 0.0f});
    v.push_back({c.x, c.y, c.z, n.x, n.y, n.z, 1.0f, 1.0f});
    v.push_back({d.x, d.y, d.z, n.x, n.y, n.z, 0.0f, 1.0f});
    idx.push_back(base + 0);
    idx.push_back(base + 1);
    idx.push_back(base + 2);
    idx.push_back(base + 0);
    idx.push_back(base + 2);
    idx.push_back(base + 3);
}

Mesh Mesh::makeCube() {
    float p = 0.5f;
    std::vector<Vertex> v;
    std::vector<uint32_t> idx;

    addFace(v, idx, {0, 0, 1},  {-p,-p, p}, { p,-p, p}, { p, p, p}, {-p, p, p});
    addFace(v, idx, {0, 0,-1},  { p,-p,-p}, {-p,-p,-p}, {-p, p,-p}, { p, p,-p});
    addFace(v, idx, {1, 0, 0},  { p,-p, p}, { p,-p,-p}, { p, p,-p}, { p, p, p});
    addFace(v, idx, {-1,0, 0},  {-p,-p,-p}, {-p,-p, p}, {-p, p, p}, {-p, p,-p});
    addFace(v, idx, {0, 1, 0},  {-p, p, p}, { p, p, p}, { p, p,-p}, {-p, p,-p});
    addFace(v, idx, {0,-1, 0},  {-p,-p,-p}, { p,-p,-p}, { p,-p, p}, {-p,-p, p});

    Mesh m;
    m.upload(v, idx);
    return m;
}

Mesh Mesh::makeSphere(int stacks, int slices) {
    std::vector<Vertex> v;
    std::vector<uint32_t> idx;
    float PI = 3.14159265358979f;

    for (int i = 0; i <= stacks; ++i) {
        float phi = PI * (float)i / (float)stacks;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * PI * (float)j / (float)slices;
            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);
            Vertex vert;
            vert.px = x * 0.5f; vert.py = y * 0.5f; vert.pz = z * 0.5f;
            vert.nx = x; vert.ny = y; vert.nz = z;
            vert.u = (float)j / (float)slices;
            vert.v = (float)i / (float)stacks;
            v.push_back(vert);
        }
    }

    int cols = slices + 1;
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            uint32_t a = i * cols + j;
            uint32_t b = (i + 1) * cols + j;
            idx.push_back(a);     idx.push_back(b);     idx.push_back(a + 1);
            idx.push_back(a + 1); idx.push_back(b);     idx.push_back(b + 1);
        }
    }

    Mesh m;
    m.upload(v, idx);
    return m;
}

Mesh Mesh::makeFullscreenQuad() {
    std::vector<Vertex> v = {
        {-1,-1, 0, 0, 0, 1, 0, 0},
        { 1,-1, 0, 0, 0, 1, 1, 0},
        { 1, 1, 0, 0, 0, 1, 1, 1},
        {-1, 1, 0, 0, 0, 1, 0, 1},
    };
    std::vector<uint32_t> idx = {0, 1, 2, 0, 2, 3};
    Mesh m;
    m.upload(v, idx);
    return m;
}
