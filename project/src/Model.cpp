#include "Model.h"
#include "Texture.h"
#include "tiny_obj_loader.h"
#include <cstdio>
#include <map>
#include <tuple>
#include <cmath>

// Les couleurs du .mtl sont considerees comme sRGB -> on les passe en lineaire
// pour faire l'eclairage en espace lineaire (cf. note gamma du sujet).
static Vec3 srgbToLinear(Vec3 c) {
    return { std::pow(c.x, 2.2f), std::pow(c.y, 2.2f), std::pow(c.z, 2.2f) };
}

static std::string directoryOf(const std::string& path) {
    size_t s = path.find_last_of("/\\");
    return (s == std::string::npos) ? std::string(".") : path.substr(0, s);
}

bool Model::load(const std::string& objPath) {
    tinyobj::ObjReaderConfig config;
    config.triangulate = true;                 // force les triangles
    config.mtl_search_path = directoryOf(objPath);

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(objPath, config)) {
        std::fprintf(stderr, "[Model] echec lecture %s : %s\n",
                     objPath.c_str(), reader.Error().c_str());
        return false;
    }
    if (!reader.Warning().empty())
        std::fprintf(stderr, "[Model] warning : %s\n", reader.Warning().c_str());

    const auto& attrib    = reader.GetAttrib();
    const auto& shapes    = reader.GetShapes();
    const auto& materials = reader.GetMaterials();
    const std::string dir = directoryOf(objPath);

    // --- Prepare un Material OpenGL par materiau du .mtl ---
    auto buildMaterial = [&](int mid) -> Material {
        Material out;
        if (mid < 0 || mid >= (int)materials.size()) return out; // defaut
        const tinyobj::material_t& m = materials[mid];
        out.ambient   = srgbToLinear({m.ambient[0],  m.ambient[1],  m.ambient[2]});
        out.diffuse   = srgbToLinear({m.diffuse[0],  m.diffuse[1],  m.diffuse[2]});
        out.specular  = { m.specular[0], m.specular[1], m.specular[2] }; // specular reste lineaire
        out.shininess = (m.shininess > 0.0f) ? m.shininess : 32.0f;
        if (!m.diffuse_texname.empty()) {
            out.diffuseTex = Texture::loadFromFile(dir + "/" + m.diffuse_texname, true);
            out.hasDiffuseTex = (out.diffuseTex != 0);
        }
        return out;
    };

    // On regroupe la geometrie par materiau (une ModelPart par materiau).
    struct Bucket {
        std::vector<Vertex> verts;
        std::vector<uint32_t> indices;
        std::map<std::tuple<int,int,int>, uint32_t> seen; // FUSION des triplets
    };
    std::map<int, Bucket> buckets; // cle = material id (-1 = aucun)

    for (const auto& shape : shapes) {
        size_t indexOffset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            int fv = shape.mesh.num_face_vertices[f]; // = 3 (triangule)
            int mid = shape.mesh.material_ids.empty() ? -1 : shape.mesh.material_ids[f];
            Bucket& bk = buckets[mid];

            for (int v = 0; v < fv; ++v) {
                tinyobj::index_t i = shape.mesh.indices[indexOffset + v];
                std::tuple<int,int,int> key{ i.vertex_index, i.normal_index, i.texcoord_index };

                auto it = bk.seen.find(key);
                if (it != bk.seen.end()) {
                    bk.indices.push_back(it->second); // deja vu -> reutilise l'indice
                    continue;
                }

                Vertex vert{};
                vert.px = attrib.vertices[3*i.vertex_index + 0];
                vert.py = attrib.vertices[3*i.vertex_index + 1];
                vert.pz = attrib.vertices[3*i.vertex_index + 2];

                if (i.normal_index >= 0) {
                    vert.nx = attrib.normals[3*i.normal_index + 0];
                    vert.ny = attrib.normals[3*i.normal_index + 1];
                    vert.nz = attrib.normals[3*i.normal_index + 2];
                } else {
                    vert.nx = 0; vert.ny = 1; vert.nz = 0; // a defaut (voir TODO ci-dessous)
                }

                if (i.texcoord_index >= 0) {
                    vert.u = attrib.texcoords[2*i.texcoord_index + 0];
                    vert.v = attrib.texcoords[2*i.texcoord_index + 1];
                }

                uint32_t newIndex = (uint32_t)bk.verts.size();
                bk.verts.push_back(vert);
                bk.indices.push_back(newIndex);
                bk.seen.emplace(key, newIndex);
            }
            indexOffset += fv;
        }
    }
    // TODO (option) : si le .obj n'a pas de normales, les recalculer ici
    //                 (moyenne des normales de faces) avant l'upload.

    parts_.clear();
    for (auto& [mid, bk] : buckets) {
        if (bk.verts.empty()) continue;
        ModelPart part;
        part.mesh = std::make_unique<Mesh>();
        part.mesh->upload(bk.verts, bk.indices);
        part.material = buildMaterial(mid);
        parts_.push_back(std::move(part));
    }
    std::printf("[Model] %s charge : %zu part(s)\n", objPath.c_str(), parts_.size());
    return !parts_.empty();
}
