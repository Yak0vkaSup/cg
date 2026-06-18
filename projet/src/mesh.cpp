#include "mesh.h"
#include "gl_utils.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <cstdio>
#include <map>
#include <unordered_map>

namespace {
struct V { float px, py, pz, nx, ny, nz, u, v; };
}

bool Mesh::load(const std::string& objPath, const std::string& assetDir) {
    tinyobj::ObjReaderConfig config;
    config.mtl_search_path = assetDir;
    config.triangulate = true;

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(objPath, config)) {
        std::fprintf(stderr, "[obj] failed to load %s: %s\n",
                     objPath.c_str(), reader.Error().c_str());
        return false;
    }
    if (!reader.Warning().empty())
        std::fprintf(stderr, "[obj warn] %s", reader.Warning().c_str());

    const tinyobj::attrib_t&               attrib   = reader.GetAttrib();
    const std::vector<tinyobj::shape_t>&   shapes   = reader.GetShapes();
    const std::vector<tinyobj::material_t>& objMats = reader.GetMaterials();

    std::unordered_map<std::string, GLuint> texCache;
    auto loadTex = [&](const std::string& file) -> GLuint {
        if (file.empty()) return 0;
        auto it = texCache.find(file);
        if (it != texCache.end()) return it->second;
        GLuint t = glu::load_texture_2d(assetDir + "/" + file, true);
        texCache[file] = t;
        return t;
    };

    for (const tinyobj::material_t& m : objMats) {
        Material mat;
        mat.name = m.name;
        mat.Ka = { m.ambient[0],  m.ambient[1],  m.ambient[2]  };
        mat.Kd = { m.diffuse[0],  m.diffuse[1],  m.diffuse[2]  };
        mat.Ks = { m.specular[0], m.specular[1], m.specular[2] };
        mat.Ns = m.shininess > 0.0f ? m.shininess : 32.0f;

        if (mat.name.find("metal") != std::string::npos ||
            mat.name.find("gold")  != std::string::npos ||
            mat.name.find("chrome")!= std::string::npos)
            mat.metallic = 1.0f;
        if (!m.diffuse_texname.empty()) {
            mat.diffuseTex = loadTex(m.diffuse_texname);
            mat.hasTexture = mat.diffuseTex != 0;
        }
        materials_.push_back(mat);
    }

    const int defaultMat = (int)materials_.size();
    materials_.push_back(Material{});

    std::map<int, std::vector<V>> byMat;
    float maxR2 = 0.0f;

    for (const tinyobj::shape_t& shape : shapes) {
        const tinyobj::mesh_t& mesh = shape.mesh;
        size_t indexOffset = 0;
        for (size_t f = 0; f < mesh.num_face_vertices.size(); ++f) {
            int fv = mesh.num_face_vertices[f];
            int matId = mesh.material_ids[f];
            if (matId < 0 || matId >= (int)objMats.size()) matId = defaultMat;

            V tri[3];
            for (int k = 0; k < fv && k < 3; ++k) {
                tinyobj::index_t idx = mesh.indices[indexOffset + k];
                V vert{};
                vert.px = attrib.vertices[3 * idx.vertex_index + 0];
                vert.py = attrib.vertices[3 * idx.vertex_index + 1];
                vert.pz = attrib.vertices[3 * idx.vertex_index + 2];
                if (idx.normal_index >= 0) {
                    vert.nx = attrib.normals[3 * idx.normal_index + 0];
                    vert.ny = attrib.normals[3 * idx.normal_index + 1];
                    vert.nz = attrib.normals[3 * idx.normal_index + 2];
                }
                if (idx.texcoord_index >= 0) {
                    vert.u = attrib.texcoords[2 * idx.texcoord_index + 0];
                    vert.v = attrib.texcoords[2 * idx.texcoord_index + 1];
                }
                float r2 = vert.px*vert.px + vert.py*vert.py + vert.pz*vert.pz;
                if (r2 > maxR2) maxR2 = r2;
                tri[k] = vert;
            }

            if (mesh.indices[indexOffset].normal_index < 0 && fv >= 3) {
                Vec3 a { tri[1].px - tri[0].px, tri[1].py - tri[0].py, tri[1].pz - tri[0].pz };
                Vec3 b { tri[2].px - tri[0].px, tri[2].py - tri[0].py, tri[2].pz - tri[0].pz };
                Vec3 n = vec3_norm(vec3_cross(a, b));
                for (int k = 0; k < 3; ++k) { tri[k].nx = n.x; tri[k].ny = n.y; tri[k].nz = n.z; }
            }
            std::vector<V>& bucket = byMat[matId];
            for (int k = 0; k < fv && k < 3; ++k) bucket.push_back(tri[k]);
            indexOffset += fv;
        }
    }
    radius_ = std::sqrt(maxR2);
    if (radius_ < 1e-4f) radius_ = 1.0f;

    std::vector<V> all;
    for (auto& kv : byMat) {
        SubMesh s;
        s.start    = (GLsizei)all.size();
        s.count    = (GLsizei)kv.second.size();
        s.material = kv.first;
        subs_.push_back(s);
        all.insert(all.end(), kv.second.begin(), kv.second.end());
    }

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, all.size() * sizeof(V), all.data(), GL_STATIC_DRAW);

    const GLsizei stride = sizeof(V);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    std::printf("[obj] %s: %zu verts, %zu submeshes, %zu materials\n",
                objPath.c_str(), all.size(), subs_.size(), materials_.size());
    return true;
}

void Mesh::setup_instancing(const std::vector<float>& instanceData, GLsizei strideFloats) {
    glBindVertexArray(vao_);
    if (!instanceVbo_) glGenBuffers(1, &instanceVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo_);
    glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(float),
                 instanceData.data(), GL_STATIC_DRAW);

    const GLsizei stride = strideFloats * (GLsizei)sizeof(float);

    for (int i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, stride,
                              (void*)(size_t)(i * 4 * sizeof(float)));
        glVertexAttribDivisor(3 + i, 1);
    }

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, stride, (void*)(size_t)(16 * sizeof(float)));
    glVertexAttribDivisor(7, 1);

    glBindVertexArray(0);
}

void Mesh::destroy() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (instanceVbo_) glDeleteBuffers(1, &instanceVbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    vao_ = vbo_ = instanceVbo_ = 0;
}
