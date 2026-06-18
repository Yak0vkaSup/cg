#include "mesh.h"
#include "gl_utils.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <cstdio>
#include <map>

namespace {
// Sommet "OpenGL" : position + normale + coordonnees de texture
struct V { float px, py, pz, nx, ny, nz, u, v; };

// Cle d'un sommet OBJ = triplet d'indices (position / normale / texcoord).
// Le format OBJ utilise 3 tableaux d'indices distincts ; OpenGL n'en gere qu'un.
// On recree donc des sommets uniques et on les deduplique grace a cette cle (cf. annexe A).
struct Key {
    int v, n, t;
    bool operator<(const Key& o) const {
        if (v != o.v) return v < o.v;
        if (n != o.n) return n < o.n;
        return t < o.t;
    }
};
}

bool Mesh::load(const std::string& objPath, const std::string& assetDir) {
    tinyobj::ObjReaderConfig config;
    config.mtl_search_path = assetDir;
    config.triangulate = true; // on force la triangularisation (cf. annexe A)

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(objPath, config)) {
        std::fprintf(stderr, "[obj] echec chargement %s : %s\n",
                     objPath.c_str(), reader.Error().c_str());
        return false;
    }
    if (!reader.Warning().empty())
        std::fprintf(stderr, "[obj warn] %s", reader.Warning().c_str());

    const tinyobj::attrib_t&                attrib  = reader.GetAttrib();
    const std::vector<tinyobj::shape_t>&    shapes  = reader.GetShapes();
    const std::vector<tinyobj::material_t>& objMats = reader.GetMaterials();

    // --- Lecture des materiaux du fichier MTL (Ka, Kd, Ks, Ns, map_Kd) ---
    std::map<std::string, GLuint> texCache;
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
        if (!m.diffuse_texname.empty()) {
            mat.diffuseTex = loadTex(m.diffuse_texname);
            mat.hasTexture = (mat.diffuseTex != 0);
        }
        materials_.push_back(mat);
    }
    const int defaultMat = (int)materials_.size();
    materials_.push_back(Material{}); // materiau par defaut si une face n'en a pas

    // --- Construction des sommets uniques et des indices, groupes par materiau ---
    std::vector<V>      vertices;                  // tableau de sommets (-> VBO)
    std::map<int, std::vector<GLuint>> indicesByMat; // indices, regroupes par materiau
    std::map<Key, GLuint> uniqueVerts;             // deduplication des sommets

    for (const tinyobj::shape_t& shape : shapes) {
        const tinyobj::mesh_t& mesh = shape.mesh;
        size_t indexOffset = 0;
        for (size_t f = 0; f < mesh.num_face_vertices.size(); ++f) {
            int fv = mesh.num_face_vertices[f];
            int matId = mesh.material_ids[f];
            if (matId < 0 || matId >= (int)objMats.size()) matId = defaultMat;

            for (int k = 0; k < fv && k < 3; ++k) {
                tinyobj::index_t idx = mesh.indices[indexOffset + k];
                Key key { idx.vertex_index, idx.normal_index, idx.texcoord_index };

                GLuint vertexIndex;
                auto it = uniqueVerts.find(key);
                if (it != uniqueVerts.end()) {
                    vertexIndex = it->second; // sommet deja cree : on le reutilise
                } else {
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
                    vertexIndex = (GLuint)vertices.size();
                    vertices.push_back(vert);
                    uniqueVerts[key] = vertexIndex;
                }
                indicesByMat[matId].push_back(vertexIndex);
            }
            indexOffset += fv;
        }
    }

    // Concatenation des indices de chaque materiau dans un seul tableau (-> IBO)
    std::vector<GLuint> indices;
    for (auto& kv : indicesByMat) {
        SubMesh s;
        s.indexStart = (GLsizei)indices.size();
        s.indexCount = (GLsizei)kv.second.size();
        s.material   = kv.first;
        subs_.push_back(s);
        indices.insert(indices.end(), kv.second.begin(), kv.second.end());
    }

    // --- Envoi vers le GPU : VAO + VBO (sommets) + IBO (indices) ---
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ibo_);
    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(V), vertices.data(), GL_STATIC_DRAW);

    // L'IBO doit etre lie pendant que le VAO est actif (il fait partie de l'etat du VAO)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                 indices.data(), GL_STATIC_DRAW);

    const GLsizei stride = sizeof(V);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);                  // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));  // normale
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6*sizeof(float)));  // coord. texture
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    std::printf("[obj] %s : %zu sommets, %zu indices, %zu sous-maillages\n",
                objPath.c_str(), vertices.size(), indices.size(), subs_.size());
    return true;
}

void Mesh::setup_instancing(const std::vector<float>& instanceData, GLsizei strideFloats) {
    glBindVertexArray(vao_);
    if (!instanceVbo_) glGenBuffers(1, &instanceVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVbo_);
    glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(float),
                 instanceData.data(), GL_STATIC_DRAW);

    const GLsizei stride = strideFloats * (GLsizei)sizeof(float);

    // Une mat4 occupe 4 emplacements d'attribut consecutifs (locations 3,4,5,6)
    for (int i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, stride,
                              (void*)(size_t)(i * 4 * sizeof(float)));
        glVertexAttribDivisor(3 + i, 1); // une valeur par instance (3.b)
    }
    // Couleur de l'instance (location 7)
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, stride, (void*)(size_t)(16 * sizeof(float)));
    glVertexAttribDivisor(7, 1);

    glBindVertexArray(0);
}

void Mesh::destroy() {
    if (vbo_)         glDeleteBuffers(1, &vbo_);
    if (ibo_)         glDeleteBuffers(1, &ibo_);
    if (instanceVbo_) glDeleteBuffers(1, &instanceVbo_);
    if (vao_)         glDeleteVertexArrays(1, &vao_);
    vao_ = vbo_ = ibo_ = instanceVbo_ = 0;
}
