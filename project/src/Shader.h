#pragma once
#include "gl_common.h"
#include <string>

class Shader {
public:
    bool load(const std::string& vertPath, const std::string& fragPath);
    void destroy();

    void use() const;
    GLuint id() const { return program_; }

    void setInt(const std::string& name, int v) const;
    void setFloat(const std::string& name, float v) const;
    void setVec3(const std::string& name, Vec3 v) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setMat4(const std::string& name, const Mat4& m) const;

    void bindUniformBlock(const std::string& blockName, GLuint bindingPoint) const;

private:
    GLuint program_ = 0;
    int loc(const std::string& name) const;
};
