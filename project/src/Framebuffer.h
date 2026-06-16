#pragma once
// =============================================================
//  OWNER : Personne C (Pipeline)
//  FBO de rendu hors ecran (exigence 1.d) : on dessine la scene
//  dedans (texture couleur + depth), puis on recopie cette texture
//  a l'ecran via un quad plein ecran (ce qui permet le post-traitement, 3.a).
// =============================================================
#include "gl_common.h"

class Framebuffer {
public:
    void create(int width, int height);
    void resize(int width, int height);
    void destroy();

    void bind() const;          // dessiner DANS le FBO
    static void bindDefault(int width, int height); // revenir a l'ecran

    GLuint colorTexture() const { return colorTex_; }

private:
    GLuint fbo_ = 0;
    GLuint colorTex_ = 0;
    GLuint depthRbo_ = 0;
    int w_ = 0, h_ = 0;
};
