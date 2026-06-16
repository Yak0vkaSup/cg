#version 330 core
// OWNER : Personne C (Pipeline) — quad plein ecran (passe FBO -> ecran)
layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aUV;
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = vec4(aPos, 1.0); // deja en coordonnees NDC
}
