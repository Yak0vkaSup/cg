#version 410 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float uExposure;
uniform int   uPostMode;

vec3 aces(vec3 x) {
    const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 hdr = texture(uScene, vUV).rgb * uExposure;
    vec3 col = aces(hdr);

    vec2 dd = vUV - 0.5;
    float vig = smoothstep(0.9, 0.2, dot(dd, dd) * 1.7);
    col *= mix(1.0, vig, 0.55);

    if (uPostMode == 1) {
        col = vec3(dot(col, vec3(0.299, 0.587, 0.114)));
    } else if (uPostMode == 2) {
        col = vec3(1.0) - col;
    } else if (uPostMode == 3) {
        col = vec3(dot(col, vec3(0.393, 0.769, 0.189)),
                   dot(col, vec3(0.349, 0.686, 0.168)),
                   dot(col, vec3(0.272, 0.534, 0.131)));
    }

    col = pow(clamp(col, 0.0, 1.0), vec3(1.0 / 2.2));
    FragColor = vec4(col, 1.0);
}
