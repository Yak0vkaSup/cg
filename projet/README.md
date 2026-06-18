# Projet OpenGL M1 — Scène 3D & navigation

Application OpenGL 4.3 core (GLFW + glad) qui affiche une scène 3D composée de
plusieurs objets et permet d'y naviguer avec une caméra orbitale.

![rendu](docs/screenshot.png)

## Compilation

Dépendances récupérées automatiquement par CMake (`FetchContent`) : **GLFW 3.4**,
**glad 2.0.6** (gl core 4.3), **Dear ImGui 1.91.5**. Les bibliothèques
mono-header **stb_image / stb_image_write** et **TinyOBJLoader** sont fournies
dans `src/`.

```sh
cmake -S . -B build
cmake --build build --config Release
```

L'exécutable est généré dans `build/Release/projet.exe` (les dossiers `shaders/`
et `assets/` sont copiés à côté automatiquement). Il faut **OpenGL 4.3** (pour le
compute shader de la partie 3.d et les `layout(binding=…)` des UBO).

### Génération des assets

Les meshes (`.obj`/`.mtl`) et textures (`.png`) sont produits par un script —
ils sont déjà committés, mais peuvent être régénérés :

```sh
python tools/gen_assets.py
```

## Commandes

| Entrée | Action |
|--------|--------|
| glisser souris (clic gauche) | orbiter (azimut / élévation) |
| molette | distance caméra (zoom) |
| espace | pause / reprise de l'animation |
| `R` | réinitialiser la caméra |
| `P` | enregistrer une capture `screenshot.png` |
| `Échap` | quitter |

Le panneau **ImGui** permet de basculer chaque option d'éclairage et de
post-traitement en direct. Lancer avec `--screenshot` rend ~90 images puis écrit
`screenshot.png` et quitte (utile pour vérifier le rendu sans interaction).

## Couverture du cahier des charges

### Partie 1 — Affichage
- **1.a Chargement OBJ + matériaux** — `mesh.cpp` via TinyOBJLoader
  (triangulation forcée, `Ka`/`Kd`/`Ks`/`Ns`, `map_Kd`), groupage par matériau.
- **1.b Illumination directe** — Phong **et** Blinn-Phong (au choix dans l'UI),
  combinant textures (sRGB) et matériaux issus du `.mtl` — `shaders/phong.frag`.
- **1.c Illumination indirecte** — ambiante **hémisphérique** (diffuse) +
  **environment mapping** (spéculaire, réflexion de la cubemap) — `phong.frag`.
- **1.d Rendu hors-écran** — la scène est rendue dans un **FBO HDR (RGBA16F)**
  puis résolue dans le backbuffer ; **tone mapping ACES + gamma sRGB** dans
  `post.frag` (gestion linéaire ↔ sRGB).

### Partie 2 — Navigation
- **2.a** — objets placés à des positions/rotations/échelles distinctes
  (`SceneObject`, `main.cpp`).
- **2.b** — **UBO `Camera`** (binding 0 : view + projection + position) partagé
  par tous les shaders, **UBO `Object`** (binding 1 : world matrix, normal
  matrix, matériau). Caméra **orbitale** type arcball (`camera.h`).

### Partie 3 — Options (toutes implémentées)
- **3.a** Post-traitement (tone mapping, vignette, N&B / négatif / sépia) — `post.frag`
- **3.b** **Instancing** matériel (anneau de cubes, `glDrawArraysInstanced`) — `instanced.*`
- **3.c** **Skybox** cubemap — `skybox.*` (cubemap ciel procédurale)
- **3.d** **Compute shader** générant une texture procédurale (marbre/fbm) — `procedural.comp`
- **3.e** Interface **ImGui**
- **3.f** Effet **Fresnel / back-light (rim)** — `phong.frag`
- **3.g** **Fresnel de Schlick** pour équilibrer diffus / spéculaire — `phong.frag`

## Arborescence

```
projet/
├─ CMakeLists.txt
├─ src/
│  ├─ main.cpp          application, scène, UBO, FBO, boucle de rendu, ImGui
│  ├─ math3d.h          maths colonne-major (hérité du TD de préparation)
│  ├─ camera.h          caméra orbitale
│  ├─ mesh.h / mesh.cpp chargement OBJ/MTL (TinyOBJLoader) + instancing
│  ├─ gl_utils.h        shaders, FBO, textures, cubemap procédurale
│  ├─ image_io.cpp      stb_image / stb_image_write (textures, screenshot)
│  ├─ tiny_obj_loader.h, stb_image.h, stb_image_write.h  (vendored)
├─ shaders/             phong, skybox, post, instanced, procedural (compute)
├─ assets/              .obj / .mtl / .png générés
└─ tools/gen_assets.py  générateur d'assets
```
