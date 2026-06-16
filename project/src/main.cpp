// =============================================================
//  OWNER : Personne C (Pipeline) — orchestration generale.
//  Boucle de rendu :
//    1) on dessine la scene DANS le FBO (rendu hors ecran, 1.d)
//    2) on recopie la texture du FBO a l'ecran via un quad plein ecran
//       (occasion d'ajouter du post-traitement, 3.a)
// =============================================================
#include <glad/gl.h>
#include <SDL.h>
#include <cstdio>
#include <string>

#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "CameraUBO.h"
#include "Framebuffer.h"
#include "Scene.h"

static int gWidth = 1024, gHeight = 768;

int main(int /*argc*/, char** /*argv*/) {
    // --- SDL + contexte OpenGL 3.3 core ---
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window* win = SDL_CreateWindow("Scene 3D - Projet OpenGL",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        gWidth, gHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!win) { std::fprintf(stderr, "CreateWindow: %s\n", SDL_GetError()); return 1; }

    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    if (!ctx) { std::fprintf(stderr, "GL context: %s\n", SDL_GetError()); return 1; }
    SDL_GL_MakeCurrent(win, ctx);
    SDL_GL_SetSwapInterval(1); // vsync

    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        std::fprintf(stderr, "gladLoadGL failed\n");
        return 1;
    }
    std::printf("OpenGL %s\n", glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST);

    // --- Ressources ---
    Shader phong;  phong.load("shaders/phong.vert",  "shaders/phong.frag");
    Shader screen; screen.load("shaders/screen.vert", "shaders/screen.frag");

    // Lie le bloc "Camera" des shaders au point de binding de l'UBO.
    phong.bindUniformBlock("Camera", CameraUBO::BINDING);

    CameraUBO cameraUBO; cameraUBO.create();
    Framebuffer fbo;     fbo.create(gWidth, gHeight);
    Mesh quad = Mesh::makeFullscreenQuad();

    Camera camera;
    Scene scene; scene.build();

    // --- Etat souris pour la camera orbitale ---
    bool dragging = false;
    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
            else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                gWidth = e.window.data1; gHeight = e.window.data2;
                fbo.resize(gWidth, gHeight);
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) dragging = true;
            else if (e.type == SDL_MOUSEBUTTONUP   && e.button.button == SDL_BUTTON_LEFT) dragging = false;
            else if (e.type == SDL_MOUSEMOTION && dragging)
                camera.orbit(e.motion.xrel * 0.005f, e.motion.yrel * 0.005f);
            else if (e.type == SDL_MOUSEWHEEL)
                camera.zoom(-e.wheel.y * 0.5f);
        }

        float aspect = (float)gWidth / (float)gHeight;
        cameraUBO.update(camera.view(), camera.proj(aspect), camera.position());

        // ---- PASSE 1 : scene -> FBO (hors ecran) ----
        fbo.bind();
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        phong.use();
        // uniforms communs a tous les objets (lumiere + ambiant hemispherique).
        phong.setVec3("uLightDir",   vec3_norm(scene.light.direction));
        phong.setVec3("uLightColor", scene.light.color);
        phong.setVec3("uSkyColor",    scene.ambient.skyColor);
        phong.setVec3("uGroundColor", scene.ambient.groundColor);
        phong.setInt("uDiffuseTex", 0); // unite de texture 0

        for (const auto& obj : scene.objects()) {
            phong.setMat4("uWorld", obj.transform);
            phong.setVec3("uKa", obj.material.ambient);
            phong.setVec3("uKd", obj.material.diffuse);
            phong.setVec3("uKs", obj.material.specular);
            phong.setFloat("uShininess", obj.material.shininess);
            phong.setInt("uHasDiffuseTex", obj.material.hasDiffuseTex ? 1 : 0);
            if (obj.material.hasDiffuseTex) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, obj.material.diffuseTex);
            }
            obj.mesh->draw();
        }

        // ---- PASSE 2 : FBO -> ecran (+ correction gamma / post-traitement) ----
        Framebuffer::bindDefault(gWidth, gHeight);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        screen.use();
        screen.setInt("uScreen", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fbo.colorTexture());
        quad.draw();

        SDL_GL_SwapWindow(win);
    }

    cameraUBO.destroy();
    fbo.destroy();
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
