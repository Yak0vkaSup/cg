#include "Model.h"
#include "Texture.h"
#include "tiny_obj_loader.h"
#include <cstdio>
#include <cmath>

static Vec3 srgbToLinear(Vec3 c) {
    return { std::pow(c.x, 2.2f), std::pow(c.y, 2.2f), std::pow(c.z, 2.2f) };
}

static std::string directoryOf(const std::string& path) {
    size_t s = path.find_last_of("/\\");
    if (s == std::string::npos) return ".";
    return path.substr(0, s);
}

bool Model::load(const std::string& objPath) {
    tinyobj::ObjReaderConfig config;
    config.triangulate = true;
    config.mtl_search_path = directoryOf(objPath);

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(objPath, config)) {
        std::fprintf(stderr, "Model: echec lecture %s : %s\n",
                     objPath.c_str(), reader.Error().c_str());
        return false;
    }

    const tinyobj::attrib_t& attrib = reader.GetAttrib();
    const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();
    const std::vector<tinyobj::material_t>& materials = reader.GetMaterials();
    std::string dir = directoryOf(objPath);

    int materialCount = (int)materials.size();
    std::vector<std::vector<Vertex>> bucketVerts(materialCount + 1);

    for (size_t s = 0; s < shapes.size(); ++s) {
        const tinyobj::mesh_t& mesh = shapes[s].mesh;
        size_t indexOffset = 0;
        for (size_t f = 0; f < mesh.num_face_vertices.size(); ++f) {
            int fv = mesh.num_face_vertices[f];
            int mid = mesh.material_ids.empty() ? -1 : mesh.material_ids[f];
            int bucket = (mid < 0) ? materialCount : mid;

            for (int k = 0; k < fv; ++k) {
                tinyobj::index_t i = mesh.indices[indexOffset + k];

                Vertex vert;
                vert.px = attrib.vertices[3 * i.vertex_index + 0];
                vert.py = attrib.vertices[3 * i.vertex_index + 1];
                vert.pz = attrib.vertices[3 * i.vertex_index + 2];

                if (i.normal_index >= 0) {
                    vert.nx = attrib.normals[3 * i.normal_index + 0];
                    vert.ny = attrib.normals[3 * i.normal_index + 1];
                    vert.nz = attrib.normals[3 * i.normal_index + 2];
                } else {
                    vert.nx = 0.0f; vert.ny = 1.0f; vert.nz = 0.0f;
                }

                if (i.texcoord_index >= 0) {
                    vert.u = attrib.texcoords[2 * i.texcoord_index + 0];
                    vert.v = attrib.texcoords[2 * i.texcoord_index + 1];
                } else {
                    vert.u = 0.0f; vert.v = 0.0f;
                }

                bucketVerts[bucket].push_back(vert);
            }
            indexOffset += fv;
        }
    }

    parts_.clear();
    for (int b = 0; b <= materialCount; ++b) {
        std::vector<Vertex>& verts = bucketVerts[b];
        if (verts.empty()) continue;

        std::vector<uint32_t> indices(verts.size());
        for (size_t i = 0; i < verts.size(); ++i) indices[i] = (uint32_t)i;

        Material mat;
        if (b < materialCount) {
            const tinyobj::material_t& m = materials[b];
            mat.ambient   = srgbToLinear({ m.ambient[0],  m.ambient[1],  m.ambient[2] });
            mat.diffuse   = srgbToLinear({ m.diffuse[0],  m.diffuse[1],  m.diffuse[2] });
            mat.specular  = { m.specular[0], m.specular[1], m.specular[2] };
            mat.shininess = (m.shininess > 0.0f) ? m.shininess : 32.0f;
            if (!m.diffuse_texname.empty()) {
                mat.diffuseTex = Texture::loadFromFile(dir + "/" + m.diffuse_texname, true);
                mat.hasDiffuseTex = (mat.diffuseTex != 0);
            }
        }

        ModelPart part;
        part.mesh.upload(verts, indices);
        part.material = mat;
        parts_.push_back(part);
    }

    std::printf("Model: %s charge (%zu parties)\n", objPath.c_str(), parts_.size());
    return !parts_.empty();
}
