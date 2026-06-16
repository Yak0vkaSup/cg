#pragma once
// =============================================================
//  OWNER : Personne A (Geometrie / Textures)
//  Charge une image (jpg/png...) en texture OpenGL via stb_image.
//  IMPORTANT gamma : une texture COULEUR (albedo) est en sRGB ->
//  on la charge en GL_SRGB8_ALPHA8 pour que le GPU la linearise
//  automatiquement a la lecture. Une texture de donnees (normal map,
//  rugosite...) doit rester LINEAIRE (sRGB = false).
// =============================================================
#include "gl_common.h"
#include <string>

class Texture {
public:
    // Renvoie l'id GL (0 si echec). sRGB=true pour les textures couleur.
    static GLuint loadFromFile(const std::string& path, bool sRGB = true);
};
