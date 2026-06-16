#pragma once
// Header commun : tout le monde l'inclut pour avoir OpenGL + le type Vertex.
// On inclut glad AVANT tout header OpenGL/SDL.
#include <glad/gl.h>
#include <cstddef>   // offsetof
#include "mat4.h"

// Le sommet "standard" de notre projet : position, normale, coord. de texture.
// Tous les meshes utilisent ce format -> un seul layout d'attributs partout.
//   location 0 = position (vec3)
//   location 1 = normal   (vec3)
//   location 2 = uv       (vec2)
struct Vertex {
    float px, py, pz;   // position
    float nx, ny, nz;   // normale
    float u, v;         // coordonnees de texture
};
