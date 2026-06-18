#pragma once
#include "gl_common.h"
#include <string>
#include <vector>

class Texture {
public:
    static GLuint loadFromFile(const std::string& path, bool sRGB = true);
    static GLuint loadCubemap(const std::vector<std::string>& faces, bool sRGB = true);
};
