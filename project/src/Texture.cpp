#include "Texture.h"
#include "stb_image.h"
#include <cstdio>

GLuint Texture::loadFromFile(const std::string& path, bool sRGB) {
    stbi_set_flip_vertically_on_load(true); // OpenGL : origine en bas a gauche
    int w, h, channels;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 0);
    if (!data) {
        std::fprintf(stderr, "[Texture] echec chargement %s\n", path.c_str());
        return 0;
    }

    GLenum srcFormat = GL_RGB;
    GLint  internal  = sRGB ? GL_SRGB8 : GL_RGB8;
    if (channels == 1) { srcFormat = GL_RED;  internal = GL_R8; }
    else if (channels == 3) { srcFormat = GL_RGB;  internal = sRGB ? GL_SRGB8        : GL_RGB8; }
    else if (channels == 4) { srcFormat = GL_RGBA; internal = sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8; }

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
