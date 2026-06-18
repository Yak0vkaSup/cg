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
    // On retire la translation de la vue : la skybox suit toujours la camera
    mat4 viewNoTranslation = mat4(mat3(uView));
    vec4 pos = uProj * viewNoTranslation * vec4(aPos, 1.0);
    // z = w => apres division perspective z = 1 (profondeur max : skybox au fond)
    gl_Position = pos.xyww;
}
