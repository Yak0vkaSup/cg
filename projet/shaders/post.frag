#version 410 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uScene;
uniform float     uExposure;
uniform int       uPostMode;   // 0=aucun 1=noir et blanc 2=sepia

void main() {
    vec3 hdr = texture(uScene, vUV).rgb * uExposure;

    // tone mapping de Reinhard
    vec3 color = hdr / (hdr + vec3(1.0));

    if (uPostMode == 1) {
        // noir et blanc
        color = vec3(dot(color, vec3(0.299, 0.587, 0.114)));
    } else if (uPostMode == 2) {
        // sepia
        color = vec3(dot(color, vec3(0.393, 0.769, 0.189)),
                     dot(color, vec3(0.349, 0.686, 0.168)),
                     dot(color, vec3(0.272, 0.534, 0.131)));
    }

    // correction gamma
    color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2));
    FragColor = vec4(color, 1.0);
}
