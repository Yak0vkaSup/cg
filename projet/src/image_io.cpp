#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "gl_utils.h"
#include <vector>

namespace glu {

void save_screenshot(const std::string& path, int w, int h) {
    std::vector<unsigned char> px((size_t)w * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    stbi_flip_vertically_on_write(1);
    if (stbi_write_png(path.c_str(), w, h, 3, px.data(), w * 3))
        std::printf("[screenshot] saved %s (%dx%d)\n", path.c_str(), w, h);
    else
        std::fprintf(stderr, "[screenshot] failed to write %s\n", path.c_str());
}

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

}
