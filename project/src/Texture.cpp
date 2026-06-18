#include "Texture.h"
#include "stb_image.h"
#include <cstdio>

GLuint Texture::loadFromFile(const std::string& path, bool sRGB) {
    stbi_set_flip_vertically_on_load(true);
    int w, h, channels;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 0);
    if (!data) {
        std::fprintf(stderr, "Texture: echec chargement %s\n", path.c_str());
        return 0;
    }

    GLenum srcFormat = GL_RGB;
    GLint internal = sRGB ? GL_SRGB8 : GL_RGB8;
    if (channels == 1) {
        srcFormat = GL_RED;
        internal = GL_R8;
    } else if (channels == 3) {
        srcFormat = GL_RGB;
        internal = sRGB ? GL_SRGB8 : GL_RGB8;
    } else if (channels == 4) {
        srcFormat = GL_RGBA;
        internal = sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, internal, w, h, 0, srcFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}

GLuint Texture::loadCubemap(const std::vector<std::string>& faces, bool sRGB) {
    GLuint cubemapID;
    glGenTextures(1, &cubemapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

    stbi_set_flip_vertically_on_load(false);
    for (size_t i = 0; i < faces.size(); ++i) {
        int w, h, channels;
        unsigned char* data = stbi_load(faces[i].c_str(), &w, &h, &channels, 0);
        if (!data) {
            std::fprintf(stderr, "Texture: echec chargement face %s\n", faces[i].c_str());
            continue;
        }

        GLenum srcFormat = (channels == 4) ? GL_RGBA : GL_RGB;
        GLint internal;
        if (sRGB)
            internal = (channels == 4) ? GL_SRGB8_ALPHA8 : GL_SRGB8;
        else
            internal = (channels == 4) ? GL_RGBA8 : GL_RGB8;

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (GLenum)i, 0, internal,
                     w, h, 0, srcFormat, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return cubemapID;
}
