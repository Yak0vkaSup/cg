# Projet OpenGL M1

Notre projet de Computer Graphics : une petite scène 3D (sol, sphères, tore, cube,
un anneau de cubes) avec une caméra qu'on peut bouger autour, en OpenGL moderne.

## Compilation

On utilise CMake. GLFW et ImGui sont téléchargés automatiquement, glad est déjà
fourni dans `external/glad`.

```
cmake -S . -B build
cmake --build build
```

L'exécutable se trouve dans `build/` (les dossiers `shaders/` et `assets/` sont
copiés à côté). Il faut OpenGL 4.1 minimum.

## Commandes

- clic gauche + glisser : tourner autour de la scène
- molette : zoom
- R : reset caméra
- Echap : quitter

Le panneau ImGui en haut à gauche permet d'activer/désactiver les options
(Phong/Blinn, env map, ambiante, rim, skybox, instancing, effets...).

## Ce qu'on a fait

Partie 1 :
- chargement de .obj avec TinyOBJLoader + matériaux du .mtl (mesh.cpp)
- éclairage Phong / Blinn-Phong avec textures (phong.frag)
- ambiante hémisphérique + reflets avec l'environment map
- rendu dans un FBO puis correction gamma (post.frag)

Partie 2 :
- chaque objet a sa position/rotation/échelle
- matrices envoyées aux shaders via des UBO
- caméra orbitale (camera.h)

Partie 3 (options) :
- post-traitement (noir et blanc, sépia)
- instancing pour l'anneau de cubes
- skybox en cubemap (6 images dans assets/)
- texture procédurale : compute shader si OpenGL 4.3, sinon version fragment
  (sur Mac on est limité à 4.1, du coup c'est le fallback qui tourne)
- interface ImGui
- effet de rim / back-light
- Fresnel de Schlick pour équilibrer diffus et spéculaire

## Notes

Les assets (obj/mtl/textures) sont générés par `tools/gen_assets.py`, ils sont
déjà dans le dépôt donc pas besoin de le relancer.
