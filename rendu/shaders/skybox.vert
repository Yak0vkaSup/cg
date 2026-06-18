#version 410 core
layout(location = 0) in vec3 aPos;

layout(std140) uniform Camera {
    mat4 uView;
    mat4 uProj;
    vec4 uCamPos;
};

out vec3 vDir;

void main() {
    vDir = aPos;
    mat4 viewNoTranslation = mat4(mat3(uView));  // on enleve la translation
    vec4 pos = uProj * viewNoTranslation * vec4(aPos, 1.0);
    gl_Position = pos.xyww;   // profondeur max (skybox au fond)
}
