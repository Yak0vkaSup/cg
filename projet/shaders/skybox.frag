#version 430 core
in vec3 vDir;
out vec4 FragColor;

uniform samplerCube uEnvMap;

void main() {

    FragColor = vec4(texture(uEnvMap, normalize(vDir)).rgb, 1.0);
}
