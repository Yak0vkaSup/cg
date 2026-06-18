#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "gl_utils.h"

namespace glu {

GLuint load_texture_2d(const std::string& path, bool srgb) {
    int w, h, comp;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &comp, 4);
    if (!data) {
        std::fprintf(stderr, "[texture] cannot load %s (%s)\n",
                     path.c_str(), stbi_failure_reason());
        return white_texture();
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    GLenum internal = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
    glTexImage2D(GL_TEXTURE_2D, 0, internal, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);
    std::printf("[texture] %s (%dx%d, %s)\n", path.c_str(), w, h, srgb ? "sRGB" : "linear");
    return tex;
}

GLuint load_cubemap(const std::string& dir, const std::string& ext, bool srgb) {
    GLuint cube;
    glGenTextures(1, &cube);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cube);

    // ordre des 6 faces du cubemap
    const char* names[6] = { "px", "nx", "py", "ny", "pz", "nz" };
    GLenum internal = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;

    // les faces de cubemap ne sont pas retournees verticalement
    stbi_set_flip_vertically_on_load(0);
    for (int i = 0; i < 6; ++i) {
        std::string path = dir + "/" + names[i] + ext;
        int w, h, comp;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &comp, 4);
        if (!data) {
            std::fprintf(stderr, "[cubemap] impossible de charger %s (%s)\n",
                         path.c_str(), stbi_failure_reason());
            continue;
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal,
                     w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    std::printf("[cubemap] charge depuis %s (px/nx/py/ny/pz/nz%s)\n", dir.c_str(), ext.c_str());
    return cube;
}

}
