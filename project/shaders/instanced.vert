#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in mat4 aInstanceModel;

uniform mat4 uView;
uniform mat4 uProj;

out vec3 vNormal;

void main() {
    vNormal = normalize(mat3(aInstanceModel) * aNormal);
    gl_Position = uProj * uView * aInstanceModel * vec4(aPos, 1.0);
}
