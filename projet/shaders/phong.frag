#version 430 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

layout(std140, binding = 0) uniform Camera {
    mat4 uView;
    mat4 uProj;
    vec4 uCamPos;
    vec4 uOptions;
    vec4 uOptions2;
};

layout(std140, binding = 1) uniform Object {
    mat4 uModel;
    mat4 uNormalMat;
    vec4 uKa;
    vec4 uKd;
    vec4 uKs;
    vec4 uParams;
};

uniform sampler2D   uDiffuseTex;
uniform samplerCube uEnvMap;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3  N = normalize(vNormal);
    vec3  V = normalize(uCamPos.xyz - vWorldPos);
    float NdotV = max(dot(N, V), 0.0);

    bool  hasTex   = uParams.x > 0.5;
    float metallic = uParams.y;
    bool  blinn    = uOptions.x  > 0.5;
    bool  schlick  = uOptions2.y > 0.5;

    vec3 albedo = uKd.rgb * (hasTex ? texture(uDiffuseTex, vUV).rgb : vec3(1.0));

    vec3 F0          = mix(vec3(0.04), albedo, metallic);
    vec3 diffuseCol  = albedo * (1.0 - metallic);
    vec3 specTint    = mix(vec3(1.0), uKs.rgb, metallic);
    float shininess  = max(uKs.w, 1.0);

    float t = uOptions2.x;
    vec3  keyDir = normalize(vec3(cos(t * 0.3), 0.95, sin(t * 0.3)));
    vec3  keyCol = vec3(2.1, 2.05, 1.95);

    vec3  fillPos  = vec3(-5.0, 4.0, -4.0);
    vec3  toFill   = fillPos - vWorldPos;
    float fillDist = length(toFill);
    vec3  fillL    = toFill / max(fillDist, 1e-3);
    vec3  fillCol  = vec3(0.9, 0.55, 0.4) * (22.0 / (fillDist * fillDist));

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 2; ++i) {
        vec3 L        = (i == 0) ? keyDir : fillL;
        vec3 radiance = (i == 0) ? keyCol : fillCol;
        float NdotL = max(dot(N, L), 0.0);
        if (NdotL <= 0.0) continue;

        vec3  H = normalize(L + V);
        float specVal = blinn
            ? pow(max(dot(N, H), 0.0), shininess)
            : pow(max(dot(reflect(-L, N), V), 0.0), shininess);

        vec3 F  = schlick ? fresnelSchlick(max(dot(H, V), 0.0), F0) : F0;
        vec3 kd = vec3(1.0) - F;
        vec3 spec = F * specTint * specVal;
        Lo += radiance * NdotL * (kd * diffuseCol + spec);
    }

    vec3 indirect = vec3(0.0);

    if (uOptions.z > 0.5) {
        vec3 sky    = vec3(0.40, 0.50, 0.72);
        vec3 ground = vec3(0.20, 0.18, 0.16);
        float h = 0.5 * (N.y + 1.0);
        indirect += mix(ground, sky, h) * diffuseCol;
    } else {
        indirect += uKa.rgb;
    }

    if (uOptions.y > 0.5) {
        vec3 R = reflect(-V, N);
        vec3 envCol = texture(uEnvMap, R).rgb;
        vec3 Fenv = fresnelSchlick(NdotV, F0);
        indirect += Fenv * specTint * envCol;
    }

    vec3 color = Lo + indirect;

    if (uOptions.w > 0.5) {
        float rimPow = max(uOptions2.w, 0.5);
        float rim = pow(1.0 - NdotV, rimPow);
        color += rim * vec3(0.25, 0.40, 0.70) * 0.6;
    }

    FragColor = vec4(color, 1.0);
}
