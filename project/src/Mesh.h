#pragma once
// =============================================================
//  OWNER : Personne A (Geometrie)
//  Un Mesh = un VAO + VBO + IBO. On lui donne des sommets + indices,
//  il sait se dessiner. Reprend directement le TP1 (VBO/IBO/VAO).
// =============================================================
#include "gl_common.h"
#include <vector>
#include <cstdint>

class Mesh {
public:
    Mesh() = default;
    ~Mesh() { destroy(); }

    // Pas de copie (un Mesh possede des objets GL), mais on autorise le move.
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& o) noexcept { *this = std::move(o); }
    Mesh& operator=(Mesh&& o) noexcept;

    // Envoie les donnees vers le GPU (cree VAO/VBO/IBO).
    void upload(const std::vector<Vertex>& vertices,
                const std::vector<uint32_t>& indices);
    void draw() const;
    void destroy();

    bool valid() const { return vao_ != 0; }

    // Primitives utiles pour demarrer / tester sans fichier OBJ.
    static Mesh makeCube();
    static Mesh makeSphere(int stacks = 24, int slices = 32);
    static Mesh makeFullscreenQuad(); // pour le rendu hors ecran (FBO -> ecran)

private:
    GLuint vao_ = 0, vbo_ = 0, ibo_ = 0;
    GLsizei count_ = 0;
};
