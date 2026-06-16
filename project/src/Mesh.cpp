#include "Mesh.h"
#include <cmath>
#include <utility>

Mesh& Mesh::operator=(Mesh&& o) noexcept {
    if (this != &o) {
        destroy();
        vao_ = o.vao_; vbo_ = o.vbo_; ibo_ = o.ibo_; count_ = o.count_;
        o.vao_ = o.vbo_ = o.ibo_ = 0; o.count_ = 0;
    }
    return *this;
}

void Mesh::upload(const std::vector<Vertex>& vertices,
                  const std::vector<uint32_t>& indices) {
    destroy();
    count_ = (GLsizei)indices.size();

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ibo_);

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
                 indices.data(), GL_STATIC_DRAW);

    // layout : 0=position, 1=normal, 2=uv  (cf. Vertex dans gl_common.h)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, px));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::draw() const {
    if (!vao_) return;
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Mesh::destroy() {
    if (ibo_) glDeleteBuffers(1, &ibo_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    vao_ = vbo_ = ibo_ = 0; count_ = 0;
}

// ---------------------------------------------------------------
// Primitives generees en code (pratique pour tester sans assets).
// ---------------------------------------------------------------
Mesh Mesh::makeCube() {
    // 24 sommets (4 par face) pour avoir des normales correctes par face.
    const float p = 0.5f;
    std::vector<Vertex> v;
    std::vector<uint32_t> idx;
    // n = normale de la face, defini 4 coins.
    auto face = [&](Vec3 n, Vec3 a, Vec3 b, Vec3 c, Vec3 d) {
        uint32_t base = (uint32_t)v.size();
        v.push_back({a.x,a.y,a.z, n.x,n.y,n.z, 0,0});
        v.push_back({b.x,b.y,b.z, n.x,n.y,n.z, 1,0});
        v.push_back({c.x,c.y,c.z, n.x,n.y,n.z, 1,1});
        v.push_back({d.x,d.y,d.z, n.x,n.y,n.z, 0,1});
        idx.insert(idx.end(), {base,base+1,base+2, base,base+2,base+3});
    };
    face({0,0,1},  {-p,-p, p},{ p,-p, p},{ p, p, p},{-p, p, p}); // +Z
    face({0,0,-1}, { p,-p,-p},{-p,-p,-p},{-p, p,-p},{ p, p,-p}); // -Z
    face({1,0,0},  { p,-p, p},{ p,-p,-p},{ p, p,-p},{ p, p, p}); // +X
    face({-1,0,0}, {-p,-p,-p},{-p,-p, p},{-p, p, p},{-p, p,-p}); // -X
    face({0,1,0},  {-p, p, p},{ p, p, p},{ p, p,-p},{-p, p,-p}); // +Y
    face({0,-1,0}, {-p,-p,-p},{ p,-p,-p},{ p,-p, p},{-p,-p, p}); // -Y
    Mesh m; m.upload(v, idx); return m;
}

Mesh Mesh::makeSphere(int stacks, int slices) {
    std::vector<Vertex> v;
    std::vector<uint32_t> idx;
    const float PI = 3.14159265358979f;
    for (int i = 0; i <= stacks; ++i) {
        float phi = PI * (float)i / (float)stacks;          // 0..PI
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * PI * (float)j / (float)slices; // 0..2PI
            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);
            // sphere unite -> position = normale
            v.push_back({x*0.5f,y*0.5f,z*0.5f, x,y,z,
                         (float)j/slices, (float)i/stacks});
        }
    }
    int cols = slices + 1;
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            uint32_t a = i*cols + j;
            uint32_t b = (i+1)*cols + j;
            idx.insert(idx.end(), {a, b, a+1,  a+1, b, b+1});
        }
    }
    Mesh m; m.upload(v, idx); return m;
}

Mesh Mesh::makeFullscreenQuad() {
    // Quad qui couvre tout l'ecran en NDC. uv dans [0,1] pour echantillonner le FBO.
    std::vector<Vertex> v = {
        {-1,-1,0, 0,0,1, 0,0},
        { 1,-1,0, 0,0,1, 1,0},
        { 1, 1,0, 0,0,1, 1,1},
        {-1, 1,0, 0,0,1, 0,1},
    };
    std::vector<uint32_t> idx = {0,1,2, 0,2,3};
    Mesh m; m.upload(v, idx); return m;
}
