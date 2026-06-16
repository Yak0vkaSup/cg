#pragma once
// =============================================================
//  OWNER : Personne A (Geometrie / Materiaux)
//  Donnees de materiau issues du fichier MTL (ou definies a la main).
//  Couleurs en espace LINEAIRE (la conversion sRGB->lineaire des
//  couleurs MTL se fait au chargement, cf. Model.cpp).
// =============================================================
#include "gl_common.h"

struct Material {
    Vec3  ambient   = {0.1f, 0.1f, 0.1f}; // Ka
    Vec3  diffuse   = {0.8f, 0.8f, 0.8f}; // Kd
    Vec3  specular  = {0.5f, 0.5f, 0.5f}; // Ks
    float shininess = 32.0f;              // Ns

    // Texture diffuse optionnelle (0 = pas de texture, on utilise diffuse).
    GLuint diffuseTex = 0;
    bool   hasDiffuseTex = false;
};
