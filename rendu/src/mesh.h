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
    GLuint diffuseTex = 0;
    bool   hasTexture = false;
    std::string name;
};


struct SubMesh {
    GLsizei indexStart = 0;
    GLsizei indexCount = 0;
    int     material   = 0;
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
            glDrawElements(GL_TRIANGLES, s.indexCount, GL_UNSIGNED_INT,
                           (const void*)(s.indexStart * sizeof(GLuint)));
        }
    }

    void draw_instanced(GLsizei instanceCount) const {
        glBindVertexArray(vao_);
        for (const SubMesh& s : subs_)
            glDrawElementsInstanced(GL_TRIANGLES, s.indexCount, GL_UNSIGNED_INT,
                                    (const void*)(s.indexStart * sizeof(GLuint)),
                                    instanceCount);
    }

    void setup_instancing(const std::vector<float>& instanceData, GLsizei strideFloats);

    GLuint vao() const { return vao_; }
    const std::vector<Material>& materials() const { return materials_; }

private:
    GLuint vao_ = 0, vbo_ = 0, ibo_ = 0, instanceVbo_ = 0;
    std::vector<SubMesh> subs_;
    std::vector<Material> materials_;
};
