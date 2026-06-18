#version 410 core

// Quad plein ecran classique (2 triangles), attributs fournis par un VBO
// (methode vue en TD : on ne reconstruit pas les sommets a partir de gl_VertexID).
layout(location = 0) in vec2 aPos;  // position en coordonnees ecran (NDC)
layout(location = 1) in vec2 aUV;   // coordonnees de texture

out vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
