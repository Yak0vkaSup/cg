# Projet OpenGL M1 — socle

Scène 3D multi-objets navigable, OpenGL 3.3 core + SDL2. Le socle couvre **toute la Partie 1 (Affichage)** et **toute la Partie 2 (Navigation)** du sujet. La Partie 3 (options) est préparée via des points d'extension (`TODO`) à remplir.

## Compiler et lancer

```
cd project
cmake -S . -B build
cmake --build build --config Release
./build/scene3d        # (Windows : build\Release\scene3d.exe)
```

CMake télécharge SDL2 et Glad automatiquement (FetchContent, nécessite Internet au premier build + Python avec `jinja2` pour Glad). TinyOBJLoader et stb_image sont déjà fournis dans `third_party/`.

Contrôles : clic gauche maintenu = orbiter, molette = zoom, Échap = quitter.

## Ce qui marche déjà

- Rendu hors écran dans un **FBO**, recopié à l'écran par un quad plein écran (1.d).
- **Blinn-Phong** : diffus + spéculaire, lumière directionnelle (1.b).
- **Ambiant hémisphérique** (diffus indirect, 1.c).
- **Correction gamma** sRGB↔linéaire (éclairage linéaire, gamma à la passe écran).
- Chargement **OBJ + MTL** avec fusion des triplets d'indices (1.a).
- **UBO** partagé pour view/proj/position caméra (2.b).
- Caméra **orbitale** + plusieurs objets transformés (2.a).

## Architecture (1 owner par fichier = travail indépendant)

| Fichier | Owner | Rôle |
|---|---|---|
| `src/Shader.*` | B | charge/compile GLSL, envoie les uniforms |
| `shaders/phong.*` | B | Blinn-Phong + ambiant + textures |
| `src/Mesh.*` | A | VAO/VBO/IBO + primitives (cube, sphère, quad) |
| `src/Model.*` | A | chargement OBJ/MTL (fusion d'indices) |
| `src/Texture.*`, `src/Material.h` | A | textures (sRGB) et matériaux |
| `src/Camera.*` | C | caméra orbitale → matrices view/proj |
| `src/CameraUBO.*` | C | UBO std140 des matrices |
| `src/Framebuffer.*` | C | FBO hors écran |
| `shaders/screen.*` | C | quad plein écran + gamma + post-traitement |
| `src/main.cpp` | C | boucle de rendu (orchestration) |
| `src/Scene.*` | **partagé** | **point d'assemblage : chacun ajoute ses objets** |

## Travailler à 3 et fusionner sans douleur

Règles simples qui évitent 90 % des conflits git :

1. **Chacun ne modifie que SES fichiers.** Les interfaces (`.h`) sont figées au départ : on code contre elles. Si tu dois changer un `.h` partagé, préviens le groupe.
2. **Un seul fichier vraiment partagé : `src/Scene.cpp`.** Il contient trois blocs `SECTION PERSONNE A / B / C`. Chacun n'écrit que dans son bloc → les diffs ne se chevauchent pas.
3. **Nouveau fichier `.cpp` ?** Ajoute-le à la liste `add_executable(...)` dans `CMakeLists.txt` (seule ligne du CMake à toucher).
4. Branches git par personne (`feat/geometrie`, `feat/shading`, `feat/pipeline`), merge fréquent vers `main`.

## Où implémenter les options (Partie 3)

Les hooks sont déjà marqués `// TODO` dans le code :

- **3.a Post-traitement** → `shaders/screen.frag` (la texture du FBO est déjà là).
- **3.c Skybox / 1.c env map spéc.** → nouveau `CubeMap` + `// TODO` dans `phong.frag` (réflexion). Vos images sont dans `../envmaps/`.
- **3.b Instancing** → ajouter un VBO d'instances dans `Mesh` + `glDrawElementsInstanced`.
- **3.f/3.g Fresnel / back-lighting** → `phong.frag` (`// TODO Fresnel`).
- **3.d Compute shader / 3.e ImGui** → modules à part (les plus coûteux, à faire en dernier).
