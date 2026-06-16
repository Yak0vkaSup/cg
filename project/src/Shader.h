#pragma once
// =============================================================
//  OWNER : Personne B (Shading)
//  Charge / compile / linke un programme GLSL et envoie les uniforms.
//  Reprend make_program du TP, generalise en classe.
// =============================================================
#include "gl_common.h"
#include <string>

class Shader {
public:
    Shader() = default;
    // Charge un vertex + fragment shader depuis des fichiers.
    bool load(const std::string& vertPath, const std::string& fragPath);
    void destroy();

    void use() const;
    GLuint id() const { return program_; }

    // Helpers d'uniforms (par nom). Pratiques et lisibles pour un projet etudiant.
    void setInt(const std::string& name, int v) const;
    void setFloat(const std::string& name, float v) const;
    void setVec3(const std::string& name, Vec3 v) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setMat4(const std::string& name, const Mat4& m) const;

    // Lie un bloc uniform (ex: "Camera") a un point de binding (ex: 0) pour les UBO.
    void bindUniformBlock(const std::string& blockName, GLuint bindingPoint) const;

private:
    GLuint program_ = 0;
    int loc(const std::string& name) const; // glGetUniformLocation
};
