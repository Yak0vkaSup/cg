#pragma once
// =============================================================
//  OWNER : Personne C (Pipeline)
//  UBO qui transfere view + proj + position camera a TOUS les shaders
//  (exigence 2.b du sujet). Un seul upload par frame, partage par
//  tous les programmes qui declarent le bloc "Camera".
//
//  ATTENTION std140 : un vec3 est aligne comme un vec4 ! On stocke
//  donc camPos en vec4 (le .w est ignore). Cote shader, le bloc doit
//  matcher EXACTEMENT :
//      layout(std140) uniform Camera {
//          mat4 uView;    // offset 0
//          mat4 uProj;    // offset 64
//          vec4 uCamPos;  // offset 128
//      };
// =============================================================
#include "gl_common.h"

class CameraUBO {
public:
    static constexpr GLuint BINDING = 0; // point de binding partage

    void create();
    void destroy();
    // Met a jour le contenu (a appeler une fois par frame).
    void update(const Mat4& view, const Mat4& proj, Vec3 camPos);

private:
    GLuint ubo_ = 0;
};
