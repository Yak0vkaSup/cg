#pragma once
#include "Mesh.h"
#include "Material.h"
#include <string>
#include <vector>

struct ModelPart {
    Mesh mesh;
    Material material;
};

class Model {
public:
    bool load(const std::string& objPath);
    const std::vector<ModelPart>& parts() const { return parts_; }

private:
    std::vector<ModelPart> parts_;
};
