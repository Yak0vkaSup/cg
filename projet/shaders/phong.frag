#version 410 core

// Entrees interpolees par le rasterizer (ombrage de Phong)
in vec3 vWorldPos;   // P : position du fragment (repere monde)
in vec3 vNormal;     // normale interpolee (a renormaliser, cf. TD)
in vec2 vUV;
out vec4 FragColor;

// UBO camera (matrices + position de l'oeil E)
layout(std140) uniform Camera {
    mat4 uView;
    mat4 uProj;
    vec4 uCamPos;    // .xyz = E (position camera)
};

// --- Materiau, nomenclature du TD : Ka, Kd, Ks, Ns ---
struct Material {
    vec3  Ka;   // couleur ambiante
    vec3  Kd;   // couleur diffuse (albedo)
    vec3  Ks;   // couleur speculaire
    float Ns;   // exposant speculaire (shininess)
};
uniform Material uMaterial;

// --- Lumiere directionnelle (cf. TD illumination, partie 2) ---
struct DirLight {
    vec3 direction;  // L : direction normalisee VERS la lumiere
    vec3 color;      // Id : intensite / couleur de la lumiere
};
uniform DirLight uLights[2];
uniform int      uNumLights;

// --- Options d'eclairage : booleens explicites (plus de packing dans un vec4) ---
uniform bool  uUseBlinn;    // true = Blinn-Phong, false = Phong
uniform bool  uUseSchlick;  // 3.g : equilibrage diffus / speculaire (Fresnel de Schlick)
uniform bool  uUseHemi;     // 1.c : ambiante hemispherique
uniform bool  uUseEnv;      // 1.c : environment mapping (speculaire indirect)
uniform bool  uUseRim;      // 3.f : effet de contour (rim / back-light)
uniform float uRimPower;

uniform bool        uHasTexture;
uniform sampler2D   uDiffuseTex;
uniform samplerCube uEnvMap;

// Approximation de Fresnel de Schlick.
// F0 = reflectance a incidence normale : on utilise Ks du materiau (cf. TD).
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // Renormalisation de la normale : l'interpolation lineaire la denormalise (cf. TD)
    vec3  N = normalize(vNormal);
    // V : direction du fragment VERS la camera
    vec3  V = normalize(uCamPos.xyz - vWorldPos);
    float NdotV = max(dot(N, V), 0.0);

    // Albedo = couleur diffuse Kd, modulee par la texture si presente
    vec3 albedo = uMaterial.Kd;
    if (uHasTexture) albedo *= texture(uDiffuseTex, vUV).rgb;

    vec3  F0        = uMaterial.Ks;          // reflectance speculaire de base
    float shininess = max(uMaterial.Ns, 1.0);

    // ===== Illumination directe : somme sur les lumieres =====
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < uNumLights; ++i) {
        vec3  L  = normalize(uLights[i].direction); // direction VERS la lumiere
        vec3  Id = uLights[i].color;                // intensite de la lumiere
        float NdotL = max(dot(N, L), 0.0);          // loi du cosinus de Lambert
        if (NdotL <= 0.0) continue;                 // face non eclairee

        vec3  H = normalize(L + V);                 // half-vector (Blinn)

        // Composante speculaire : Blinn-Phong (N.H)^Ns ou Phong (R.V)^Ns
        float specFactor;
        if (uUseBlinn) {
            specFactor = pow(max(dot(N, H), 0.0), shininess);
        } else {
            vec3 R = reflect(-L, N);                // R : reflexion de L par rapport a N
            specFactor = pow(max(dot(R, V), 0.0), shininess);
        }

        // 3.g : Fresnel de Schlick pour equilibrer diffus et speculaire.
        // F joue le role du poids speculaire (au lieu de Ks constant),
        // et kd = 1 - F assure la conservation d'energie.
        vec3 F  = uUseSchlick ? fresnelSchlick(max(dot(H, V), 0.0), F0) : F0;
        vec3 kd = vec3(1.0) - F;

        vec3 diffuse  = kd * albedo;
        vec3 specular = F  * specFactor;
        Lo += Id * NdotL * (diffuse + specular);
    }

    // ===== Illumination indirecte (ambiante) =====
    vec3 indirect = vec3(0.0);
    if (uUseHemi) {
        // 1.c : ambiante hemispherique (degrade sol -> ciel selon N.y)
        vec3 skyColor    = vec3(0.40, 0.50, 0.72);
        vec3 groundColor = vec3(0.20, 0.18, 0.16);
        float h = 0.5 * (N.y + 1.0);
        indirect += mix(groundColor, skyColor, h) * albedo;
    } else {
        // ambiante simple constante (Ka du materiau)
        indirect += uMaterial.Ka;
    }
    if (uUseEnv) {
        // 1.c : speculaire indirect par environment mapping (reflexion de la cubemap)
        vec3 R = reflect(-V, N);
        vec3 envColor = texture(uEnvMap, R).rgb;
        vec3 Fenv = uUseSchlick ? fresnelSchlick(NdotV, F0) : F0;
        indirect += Fenv * envColor;
    }

    vec3 color = Lo + indirect;

    // ===== 3.f : effet de contour (rim / back-light) =====
    if (uUseRim) {
        float rim = pow(1.0 - NdotV, max(uRimPower, 0.5));
        color += rim * vec3(0.25, 0.40, 0.70) * 0.6;
    }

    // Couleur HDR : la conversion gamma (lineaire -> sRGB) est faite dans post.frag
    FragColor = vec4(color, 1.0);
}
