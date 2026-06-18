#pragma once
#include "Mesh.h"
#include "Material.h"
#include "Model.h"
#include <vector>

struct SceneObject {
    Mesh mesh;
    Material material;
    Mat4 transform = mat4_identity();
};

struct DirLight {
    Vec3 direction = {-0.4f, -1.0f, -0.3f};
    Vec3 color     = {1.0f, 1.0f, 1.0f};
};

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
    std::vector<SceneObject> objects_;
};
