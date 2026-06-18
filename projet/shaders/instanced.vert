#version 410 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
// Donnees propres a l'instance (3.b) : matrice monde + couleur
layout(location = 3) in mat4 iModel;
layout(location = 7) in vec4 iColor;

layout(std140) uniform Camera {
    mat4 uView;
    mat4 uProj;
    vec4 uCamPos;
};

out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vColor;

void main() {
    vec4 world = iModel * vec4(aPos, 1.0);
    vWorldPos = world.xyz;
    // Normal matrix correcte = transposee de l'inverse de la matrice modele.
    // Reste juste meme si l'instance a un scale non uniforme (cf. TD illumination).
    mat3 normalMat = transpose(inverse(mat3(iModel)));
    vNormal = normalMat * aNormal;
    vColor  = iColor.rgb;
    gl_Position = uProj * uView * world;
}
