#version 120

// OpenGL GLSL
// ES 2.0 100es
//  2.1   120
//  3.2   150
//  3.3   330
//  4.6   460

// transmit par glVertexAttribPointer
// c'est une input (entree) du Vertex Shader (VS)
attribute vec2 a_Position;
attribute vec3 a_Color;

// si par contre vous utilisez glVertex2f():
// attribute vec4 gl_Vertex; (mais c'est depreciee voire obsolete)

// gl_Position est une varying predefinie de type vec4

varying vec4 v_Color; // sortie (output) du VS

void main(void)
{
    gl_Position = vec4(a_Position, 0.0, 1.0); // w=1.0 pourquoi ? cf TP2

    v_Color = vec4(a_Color, 1.0);
}
