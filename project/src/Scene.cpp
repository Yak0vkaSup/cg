#include "Scene.h"

void Scene::build() {
    // Personne A
    {
        Material violet;
        violet.diffuse = {0.55f, 0.25f, 0.8f};
        violet.specular = {0.3f, 0.3f, 0.3f};
        violet.shininess = 32.0f;
        SceneObject obj;
        obj.mesh = Mesh::makeCube();
        obj.material = violet;
        obj.transform = mat4_translation(-2.5f, 0.0f, 0.0f);
        objects_.push_back(obj);
    }

    // Personne B
    {
        Material rose;
        rose.diffuse = {0.95f, 0.4f, 0.65f};
        rose.specular = {0.5f, 0.5f, 0.5f};
        rose.shininess = 48.0f;
        SceneObject sphere;
        sphere.mesh = Mesh::makeSphere();
        sphere.material = rose;
        sphere.transform = mat4_translation(0.0f, 0.0f, 0.0f);
        objects_.push_back(sphere);

        Material bleuClair;
        bleuClair.diffuse = {0.45f, 0.75f, 0.95f};
        bleuClair.specular = {0.3f, 0.3f, 0.3f};
        bleuClair.shininess = 32.0f;
        Mat4 T = mat4_translation(2.0f, 0.0f, -1.0f);
        Mat4 S = mat4_scale(0.8f, 0.8f, 0.8f);
        SceneObject cube;
        cube.mesh = Mesh::makeCube();
        cube.material = bleuClair;
        cube.transform = mat4_mul(T, S);
        objects_.push_back(cube);
    }

    // Personne C
    {
        Material floor;
        floor.diffuse = {0.4f, 0.4f, 0.45f};
        floor.specular = {0.05f, 0.05f, 0.05f};
        floor.shininess = 8.0f;
        Mat4 T = mat4_translation(0.0f, -1.2f, 0.0f);
        Mat4 S = mat4_scale(12.0f, 0.1f, 12.0f);
        SceneObject ground;
        ground.mesh = Mesh::makeCube();
        ground.material = floor;
        ground.transform = mat4_mul(T, S);
        objects_.push_back(ground);
    }
}
