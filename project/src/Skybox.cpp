#include "Skybox.h"
#include "Texture.h"

bool Skybox::load(const std::vector<std::string>& faces) {
    cubemapTexture_ = Texture::loadCubemap(faces, true);
    skyboxCube_ = Mesh::makeCube();
    return skyboxShader_.load("shaders/skybox.vert", "shaders/skybox.frag");
}

void Skybox::draw(const Mat4& view, const Mat4& proj) {
    glDepthFunc(GL_LEQUAL);

    skyboxShader_.use();
    skyboxShader_.setMat4("uView", view);
    skyboxShader_.setMat4("uProj", proj);
    skyboxShader_.setInt("uSkybox", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture_);
    skyboxCube_.draw();

    glDepthFunc(GL_LESS);
}

void Skybox::destroy() {
    if (cubemapTexture_) glDeleteTextures(1, &cubemapTexture_);
    cubemapTexture_ = 0;
    skyboxShader_.destroy();
}
