#include "Scene.h"
#include "Texture.h"

const Mesh* Scene::keep(Mesh&& m) {
    meshes_.push_back(std::make_unique<Mesh>(std::move(m)));
    return meshes_.back().get();
}

void Scene::build() {
    // Meshes de base reutilisables.
    const Mesh* cube   = keep(Mesh::makeCube());
    const Mesh* sphere = keep(Mesh::makeSphere());

    // =========================================================
    //  SECTION PERSONNE A  (objets geometrie / OBJ / textures)
    // =========================================================
    {
        // Exemple : un cube texture (decommentez et fournissez l'image).
        // Material m;
        // m.diffuseTex = Texture::loadFromFile("assets/box.png", true);
        // m.hasDiffuseTex = (m.diffuseTex != 0);
        // objects_.push_back({ cube, m, mat4_translation(-2,0,0) });

        // Exemple : charger un .obj
        // auto model = std::make_unique<Model>();
        // if (model->load("assets/mon_objet.obj")) {
        //     Mat4 T = mat4_translation(0, 0, 2);
        //     for (auto& part : model->parts())
        //         objects_.push_back({ part.mesh.get(), part.material, T });
        //     models_.push_back(std::move(model));
        // }
    }

    // =========================================================
    //  SECTION PERSONNE B  (objets pour tester le shading)
    // =========================================================
    {
        // Sphere "metal" diffuse rouge + speculaire marque.
        Material red;
        red.diffuse = {0.7f, 0.1f, 0.1f};
        red.specular = {0.9f, 0.9f, 0.9f};
        red.shininess = 64.0f;
        objects_.push_back({ sphere, red, mat4_translation(0, 0, 0) });

        // Cube bleu mat.
        Material blue;
        blue.diffuse = {0.1f, 0.3f, 0.7f};
        blue.specular = {0.2f, 0.2f, 0.2f};
        blue.shininess = 16.0f;
        Mat4 T = mat4_translation(2.0f, 0.0f, -1.0f);
        Mat4 S = mat4_scale(0.8f, 0.8f, 0.8f);
        objects_.push_back({ cube, blue, mat4_mul(T, S) });
    }

    // =========================================================
    //  SECTION PERSONNE C  (decor / navigation)
    // =========================================================
    {
        // Sol : un cube tres aplati.
        Material floor;
        floor.diffuse = {0.4f, 0.4f, 0.45f};
        floor.specular = {0.05f, 0.05f, 0.05f};
        floor.shininess = 8.0f;
        Mat4 T = mat4_translation(0, -1.2f, 0);
        Mat4 S = mat4_scale(12.0f, 0.1f, 12.0f);
        objects_.push_back({ cube, floor, mat4_mul(T, S) });
    }
}
