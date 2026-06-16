#pragma once
// =============================================================
//  OWNER : Personne A (Geometrie)
//  Charge un .obj (+ .mtl) via TinyOBJLoader.
//  Gere LE piege du format OBJ : 3 tableaux d'indices separes
//  (position / uv / normale) -> on fusionne chaque triplet unique
//  en un seul Vertex compatible OpenGL (cf. Model.cpp).
//  Un modele est decoupe en "parts", une par materiau.
// =============================================================
#include "Mesh.h"
#include "Material.h"
#include <string>
#include <vector>
#include <memory>

struct ModelPart {
    std::unique_ptr<Mesh> mesh; // unique_ptr car Mesh n'est pas copiable
    Material material;
};

class Model {
public:
    // Charge le fichier. Renvoie false si echec (voir stderr).
    bool load(const std::string& objPath);
    const std::vector<ModelPart>& parts() const { return parts_; }

private:
    std::vector<ModelPart> parts_;
};
