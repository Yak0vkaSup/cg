#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

layout(std140) uniform Camera {
    mat4 uView;
    mat4 uProj;
    vec4 uCamPos;
};

uniform vec3  uKa;
uniform vec3  uKd;
uniform vec3  uKs;
uniform float uShininess;
uniform int   uHasDiffuseTex;
uniform sampler2D uDiffuseTex;

uniform vec3 uLightDir;
uniform vec3 uLightColor;

uniform vec3 uSkyColor;
uniform vec3 uGroundColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCamPos.xyz - vWorldPos);
    vec3 L = normalize(-uLightDir);
    vec3 H = normalize(L + V);

    vec3 albedo = uKd;
    if (uHasDiffuseTex == 1)
        albedo *= texture(uDiffuseTex, vUV).rgb;

    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = albedo * uLightColor * NdotL;

    float specAmount = 0.0;
    if (NdotL > 0.0)
        specAmount = pow(max(dot(N, H), 0.0), uShininess);
    vec3 specular = uKs * uLightColor * specAmount;

    float hemi = 0.5 * N.y + 0.5;
    vec3 ambient = albedo * mix(uGroundColor, uSkyColor, hemi);

    vec3 color = ambient + diffuse + specular;
    FragColor = vec4(color, 1.0);
}
