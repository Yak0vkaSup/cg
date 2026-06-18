#pragma once

#include <glad/gl.h>
#include <string>
#include <vector>
#include "math3d.h"

struct Material {
    Vec3   Ka { 0.05f, 0.05f, 0.05f };
    Vec3   Kd { 0.8f,  0.8f,  0.8f  };
    Vec3   Ks { 0.5f,  0.5f,  0.5f  };
    float  Ns = 32.0f;
    float  metallic = 0.0f;
    GLuint diffuseTex = 0;
    bool   hasTexture = false;
    std::string name;
};

struct SubMesh {
    GLsizei start = 0;
    GLsizei count = 0;
    int     material = 0;
};

class Mesh {
public:

    bool load(const std::string& objPath, const std::string& assetDir);
    void destroy();

    template <class Fn>
    void draw(Fn&& applyMaterial) const {
        glBindVertexArray(vao_);
        for (const SubMesh& s : subs_) {
            applyMaterial(materials_[s.material]);
            glDrawArrays(GL_TRIANGLES, s.start, s.count);
        }
    }

    void draw_instanced(GLsizei instanceCount) const {
        glBindVertexArray(vao_);
        for (const SubMesh& s : subs_)
            glDrawArraysInstanced(GL_TRIANGLES, s.start, s.count, instanceCount);
    }

    void setup_instancing(const std::vector<float>& instanceData, GLsizei strideFloats);

    GLuint vao() const { return vao_; }
    const std::vector<Material>& materials() const { return materials_; }
    float bound_radius() const { return radius_; }

private:
    GLuint vao_ = 0, vbo_ = 0, instanceVbo_ = 0;
    std::vector<SubMesh> subs_;
    std::vector<Material> materials_;
    float radius_ = 1.0f;
};
