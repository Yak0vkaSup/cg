# Projet OpenGL M1 — scène 3D navigable

Scène 3D multi-objets navigable, OpenGL 3.3 core + SDL2.

## Compiler et lancer

```
cd project
cmake -S . -B build
cmake --build build
./build/scene3d        # à lancer depuis le dossier project/
```

CMake télécharge SDL2 et Glad automatiquement (FetchContent, nécessite Internet au premier build + Python avec `jinja2` pour Glad). TinyOBJLoader et stb_image sont fournis dans `third_party/`.

Contrôles : clic gauche maintenu = orbiter, molette = zoom, Échap = quitter.

## Ce qui est fait

**Partie 1 — Affichage**

- **1.a** Chargement de modèles **OBJ + MTL** via TinyOBJLoader (couleurs ambiante/diffuse/spéculaire, texture diffuse).
- **1.b** Illumination directe **Blinn-Phong** (diffus + spéculaire), lumière directionnelle, combinée aux matériaux et textures.
- **1.c** Illumination indirecte : **ambiant hémisphérique** (diffus indirect).
- **1.d** Rendu **hors écran dans un FBO**, recopié à l'écran par un quad plein écran, avec **correction gamma** sRGB↔linéaire (éclairage en espace linéaire, gamma à la passe écran).

**Partie 2 — Navigation**

- **2.a** Plusieurs objets placés avec des translations / rotations / scales propres (cube violet, sphère rose, cube bleu clair, sol).
- **2.b** **UBO** partagé transférant les matrices vue/projection + position caméra à tous les shaders.
- Caméra **orbitale** (arcball) : orbite à la souris, zoom à la molette.

**Partie 3 — Options déjà implémentées**

- **3.c Skybox / cubemap** : chargement d'un cubemap (6 faces) et affichage d'une skybox en fond.
- **3.b Instancing** : champ de cubes affiché en un seul appel via `glDrawElementsInstanced` (matrice modèle par instance).

## Organisation des fichiers

| Fichier | Rôle |
|---|---|
| `src/Mesh.*` | VAO/VBO/IBO + primitives (cube, sphère, quad) + instancing |
| `src/Model.*` | chargement OBJ/MTL |
| `src/Texture.*`, `src/Material.h` | textures (sRGB) + cubemap, matériaux |
| `src/Shader.*` | charge/compile GLSL, envoie les uniforms |
| `src/Camera.*` | caméra orbitale → matrices vue/projection |
| `src/CameraUBO.*` | UBO std140 des matrices |
| `src/Framebuffer.*` | FBO hors écran |
| `src/Skybox.*` | skybox cubemap |
| `src/Scene.*` | objets de la scène |
| `src/main.cpp` | boucle de rendu |
| `shaders/phong.*` | Blinn-Phong + ambiant |
| `shaders/screen.*` | quad plein écran + gamma |
| `shaders/skybox.*` | skybox |
| `shaders/instanced.*` | objets instanciés |

Les images de cubemap sont dans `../envmaps/`.

## Répartition du travail (groupe de 3)

Chaque personne possède ses fichiers (interfaces `.h` figées au départ) ; seul `Scene.cpp` est partagé, découpé en blocs `// Personne A / B / C` pour éviter les conflits git.

| Personne | Fichiers possédés | Domaine |
|---|---|---|
| **A** | `Mesh.*`, `Model.*`, `Texture.*`, `Material.h`, `Skybox.*`, `shaders/skybox.*`, `shaders/instanced.*` | géométrie, OBJ, textures, buffers |
| **B** | `Shader.*`, `shaders/phong.*` | shaders, illumination |
| **C** | `Camera.*`, `CameraUBO.*`, `Framebuffer.*`, `shaders/screen.*`, `main.cpp` | caméra, UBO, FBO, pipeline |

Options de la Partie 3 réparties (~équilibrées) :

- **A** : 3.c Skybox / cubemap ✅ — 3.b Instancing ✅
- **B** : 1.c env map spéculaire (réutilise le cubemap de A) — 3.f back-lighting — 3.g Fresnel / Schlick *(à faire)*
- **C** : 3.a post-traitement — 3.e ImGui *(à faire)*
- Bonus commun éventuel : 3.d compute shader (texture procédurale)

