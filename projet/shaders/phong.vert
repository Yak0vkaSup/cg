#version 410 core

// Attributs de sommet (cf. mesh.cpp : position, normale, coord. de texture)
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

// UBO partage par tous les shaders : matrices camera + position de l'oeil (E)
layout(std140) uniform Camera {
    mat4 uView;
    mat4 uProj;
    vec4 uCamPos;   // .xyz = position de la camera (E)
};

// UBO propre a l'objet : matrice monde + normal matrix
layout(std140) uniform Object {
    mat4 uModel;      // matrice monde (local -> monde)
    mat4 uNormalMat;  // transposee de l'inverse de uModel (pour les normales)
};

out vec3 vWorldPos;  // position du fragment dans le repere monde (P)
out vec3 vNormal;    // normale (sera interpolee puis renormalisee dans le FS)
out vec2 vUV;

void main() {
    vec4 world = uModel * vec4(aPos, 1.0);
    vWorldPos  = world.xyz;
    // On transforme la normale par la "normal matrix" = (M^-1)^T (cf. TD illumination)
    vNormal    = mat3(uNormalMat) * aNormal;
    vUV        = aUV;
    gl_Position = uProj * uView * world;
}
