#version 330 core
// OWNER : Personne B (Shading)
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

// UBO partage (rempli par CameraUBO). DOIT matcher exactement le C++.
layout(std140) uniform Camera {
    mat4 uView;
    mat4 uProj;
    vec4 uCamPos;
};

uniform mat4 uWorld; // transform de l'objet (par instance)

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;

void main() {
    vec4 worldPos = uWorld * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;

    // Matrice des normales : transpose(inverse) pour rester correct
    // meme avec une mise a l'echelle non uniforme.
    mat3 normalMat = transpose(inverse(mat3(uWorld)));
    vNormal = normalize(normalMat * aNormal);

    vUV = aUV;
    gl_Position = uProj * uView * worldPos;
}
