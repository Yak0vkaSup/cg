#version 410 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vColor;
out vec4 FragColor;

layout(std140) uniform Camera {
    mat4 uView;
    mat4 uProj;
    vec4 uCamPos;
    vec4 uOptions;
    vec4 uOptions2;
};

uniform samplerCube uEnvMap;

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCamPos.xyz - vWorldPos);

    float h = 0.5 * (N.y + 1.0);
    vec3 ambient = mix(vec3(0.2, 0.18, 0.16), vec3(0.5, 0.62, 0.9), h) * vColor;

    float t = uOptions2.x;
    vec3 L = normalize(vec3(cos(t * 0.3), 0.95, sin(t * 0.3)));
    float NdotL = max(dot(N, L), 0.0);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 48.0);
    vec3 direct = vec3(2.4) * (NdotL * vColor + spec * 0.4);

    vec3 color = ambient + direct;

    if (uOptions.y > 0.5) {
        vec3 R = reflect(-V, N);
        float f = pow(1.0 - max(dot(N, V), 0.0), 4.0);
        color += f * texture(uEnvMap, R).rgb * 0.6;
    }

    FragColor = vec4(color, 1.0);
}
