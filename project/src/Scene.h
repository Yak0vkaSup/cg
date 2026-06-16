#pragma once
// =============================================================
//  FICHIER PARTAGE PAR LE GROUPE (point d'assemblage).
//  C'est ICI que chacun ajoute ses objets dans la scene, dans une
//  petite section bien identifiee, pour limiter les conflits git.
//  Structure : Personne C. Contenu : tout le monde.
// =============================================================
#include "Mesh.h"
#include "Material.h"
#include "Model.h"
#include <vector>
#include <memory>

// Un objet de la scene = une geometrie + un materiau + une transformation.
// (Le Mesh n'est PAS possede ici : il vit dans Scene, on n'a qu'un pointeur.)
struct SceneObject {
    const Mesh* mesh = nullptr;
    Material material;
    Mat4 transform = mat4_identity();
};

// Lumiere directionnelle simple (exigence 1.b).
struct DirLight {
    Vec3 direction = {-0.4f, -1.0f, -0.3f}; // direction VERS laquelle la lumiere va
    Vec3 color     = {1.0f, 1.0f, 1.0f};
};

// Couleurs pour l'ambiant hemispherique (exigence 1.c, diffus indirect).
struct HemiAmbient {
    Vec3 skyColor    = {0.5f, 0.6f, 0.8f};
    Vec3 groundColor = {0.25f, 0.2f, 0.15f};
};

class Scene {
public:
    void build();
    const std::vector<SceneObject>& objects() const { return objects_; }

    DirLight    light;
    HemiAmbient ambient;

private:
    // Stockage des ressources possedees par la scene.
    std::vector<std::unique_ptr<Mesh>>  meshes_;
    std::vector<std::unique_ptr<Model>> models_;
    std::vector<SceneObject> objects_;

    // Helper : enregistre un mesh procedural et renvoie un pointeur stable.
    const Mesh* keep(Mesh&& m);
};
