#version 410 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

layout(std140) uniform Camera {
    mat4 uView;
    mat4 uProj;
    vec4 uCamPos;   // position camera (E)
};

struct Material {
    vec3  Ka;
    vec3  Kd;
    vec3  Ks;
    float Ns;
};
uniform Material uMaterial;

// lumiere directionnelle
struct DirLight {
    vec3 direction;  // L
    vec3 color;      // Id
};
uniform DirLight uLights[2];
uniform int      uNumLights;

uniform bool  uUseBlinn;
uniform bool  uUseSchlick;
uniform bool  uUseHemi;
uniform bool  uUseEnv;
uniform bool  uUseRim;
uniform float uRimPower;

uniform bool        uHasTexture;
uniform sampler2D   uDiffuseTex;
uniform samplerCube uEnvMap;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3  N = normalize(vNormal);   // on renormalise apres interpolation
    vec3  V = normalize(uCamPos.xyz - vWorldPos);
    float NdotV = max(dot(N, V), 0.0);

    vec3 albedo = uMaterial.Kd;
    if (uHasTexture) albedo *= texture(uDiffuseTex, vUV).rgb;

    vec3  F0        = uMaterial.Ks;
    float shininess = max(uMaterial.Ns, 1.0);

    // illumination directe
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < uNumLights; ++i) {
        vec3  L  = normalize(uLights[i].direction);
        vec3  Id = uLights[i].color;
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;

        vec3  H = normalize(L + V);
        float specFactor;
        if (uUseBlinn)
            specFactor = pow(max(dot(N, H), 0.0), shininess);              // Blinn-Phong
        else
            specFactor = pow(max(dot(reflect(-L, N), V), 0.0), shininess); // Phong

        // Schlick : equilibre diffus / speculaire (kd = 1 - F)
        vec3 F  = uUseSchlick ? fresnelSchlick(max(dot(H, V), 0.0), F0) : F0;
        vec3 kd = vec3(1.0) - F;

        Lo += Id * NdotL * (kd * albedo + F * specFactor);
    }

    // ambiante : hemispherique ou simple
    vec3 indirect = vec3(0.0);
    if (uUseHemi) {
        vec3 skyColor    = vec3(0.40, 0.50, 0.72);
        vec3 groundColor = vec3(0.20, 0.18, 0.16);
        float h = 0.5 * (N.y + 1.0);
        indirect += mix(groundColor, skyColor, h) * albedo;
    } else {
        indirect += uMaterial.Ka;
    }
    // reflet de l'environnement
    if (uUseEnv) {
        vec3 R = reflect(-V, N);
        vec3 Fenv = uUseSchlick ? fresnelSchlick(NdotV, F0) : F0;
        indirect += Fenv * texture(uEnvMap, R).rgb;
    }

    vec3 color = Lo + indirect;

    // contour (rim / back-light)
    if (uUseRim) {
        float rim = pow(1.0 - NdotV, max(uRimPower, 0.5));
        color += rim * vec3(0.25, 0.40, 0.70) * 0.6;
    }

    FragColor = vec4(color, 1.0);   // HDR, gamma applique dans post.frag
}
