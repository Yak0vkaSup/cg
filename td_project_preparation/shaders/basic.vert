#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;

uniform mat4 uWorld;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vNormal;

void main() {
    mat3 normalMatrix = mat3(transpose(inverse(uWorld)));
    vNormal = normalize(normalMatrix * aNormal);
    gl_Position = uProj * uView * uWorld * vec4(aPos, 1.0);
}
