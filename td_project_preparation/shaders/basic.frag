#version 330 core
in vec3 vNormal;
out vec4 FragColor;

uniform vec3 uColor;

void main() {
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.4));
    vec3 ambient = vec3(0.25);
    float diff = max(dot(normalize(vNormal), lightDir), 0.0);
    vec3 color = uColor * (ambient + diff * 0.8);
    FragColor = vec4(color, 1.0);
}
