#version 330 core
// OWNER : Personne B (Shading)
// Eclairage fait en espace LINEAIRE. La correction gamma finale est
// appliquee dans screen.frag (passe ecran), pas ici.
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

layout(std140) uniform Camera {
    mat4 uView;
    mat4 uProj;
    vec4 uCamPos;
};

// Materiau
uniform vec3  uKa;          // ambiant
uniform vec3  uKd;          // diffus
uniform vec3  uKs;          // speculaire
uniform float uShininess;
uniform int   uHasDiffuseTex;
uniform sampler2D uDiffuseTex; // texture chargee en sRGB -> deja linearisee

// Lumiere directionnelle (1.b)
uniform vec3 uLightDir;     // direction VERS laquelle la lumiere va
uniform vec3 uLightColor;

// Ambiant hemispherique (1.c, diffus indirect)
uniform vec3 uSkyColor;
uniform vec3 uGroundColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCamPos.xyz - vWorldPos);
    vec3 L = normalize(-uLightDir);          // direction vers la lumiere
    vec3 H = normalize(L + V);               // Blinn-Phong

    // Albedo (texture si presente, sinon couleur diffuse du materiau).
    vec3 albedo = uKd;
    if (uHasDiffuseTex == 1)
        albedo *= texture(uDiffuseTex, vUV).rgb;

    // --- Contributions directes ---
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse  = albedo * uLightColor * NdotL;
    float specAmount = (NdotL > 0.0) ? pow(max(dot(N, H), 0.0), uShininess) : 0.0;
    vec3 specular = uKs * uLightColor * specAmount;

    // --- Ambiant indirect hemispherique (diffus) ---
    float hemi = 0.5 * N.y + 0.5;            // 1 vers le ciel, 0 vers le sol
    vec3 ambient = albedo * mix(uGroundColor, uSkyColor, hemi);

    // TODO (1.c spec indirect) : ajouter ici la reflexion environment map.
    // TODO (3.f/3.g) : effet Fresnel / back-lighting.

    vec3 color = ambient + diffuse + specular;
    FragColor = vec4(color, 1.0);            // lineaire (gamma fait apres)
}
