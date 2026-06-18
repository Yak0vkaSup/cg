#pragma once
#include "gl_common.h"
#include "Shader.h"
#include "Mesh.h"
#include <string>
#include <vector>

class Skybox {
public:
    bool load(const std::vector<std::string>& faces);
    void draw(const Mat4& view, const Mat4& proj);
    void destroy();

    GLuint cubemap() const { return cubemapTexture_; }

private:
    GLuint cubemapTexture_ = 0;
    Mesh skyboxCube_;
    Shader skyboxShader_;
};
