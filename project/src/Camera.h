#pragma once
// =============================================================
//  OWNER : Personne C (Pipeline / Navigation)
//  Camera orbitale : tourne autour d'une cible, zoom a la molette.
//  Reprend OrbitCam du TP. Produit les matrices view et proj.
// =============================================================
#include "mat4.h"

class Camera {
public:
    Vec3  target = {0, 0, 0};
    float radius = 6.0f;
    float phi    = 0.6f;   // azimut
    float theta  = 0.4f;   // elevation
    float fovYdeg = 60.0f;

    // Inputs (appeles depuis la gestion d'evenements).
    void orbit(float dPhi, float dTheta);
    void zoom(float dRadius);

    Vec3 position() const;
    Mat4 view() const;
    Mat4 proj(float aspect) const;
};
