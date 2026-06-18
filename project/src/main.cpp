#include <glad/gl.h>
#include <SDL.h>
#include <cstdio>
#include <string>
#include <vector>

#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"
#include "CameraUBO.h"
#include "Framebuffer.h"
#include "Scene.h"
#include "Skybox.h"

static int gWidth = 1024, gHeight = 768;

int main(int argc, char** argv) {
    (void)argc; (void)argv;

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
    if (!win) {
        std::fprintf(stderr, "CreateWindow: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    if (!ctx) {
        std::fprintf(stderr, "GL context: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_MakeCurrent(win, ctx);
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        std::fprintf(stderr, "gladLoadGL failed\n");
        return 1;
    }
    std::printf("OpenGL %s\n", glGetString(GL_VERSION));

    glEnable(GL_DEPTH_TEST);

    Shader phong;  phong.load("shaders/phong.vert",  "shaders/phong.frag");
    Shader screen; screen.load("shaders/screen.vert", "shaders/screen.frag");

    phong.bindUniformBlock("Camera", CameraUBO::BINDING);

    CameraUBO cameraUBO; cameraUBO.create();
    Framebuffer fbo;     fbo.create(gWidth, gHeight);
    Mesh quad = Mesh::makeFullscreenQuad();

    Camera camera;
    Scene scene; scene.build();

    Skybox skybox;
    std::vector<std::string> skyboxFaces = {
        "../envmaps/sky80/px.png",
        "../envmaps/sky80/nx.png",
        "../envmaps/sky80/py.png",
        "../envmaps/sky80/ny.png",
        "../envmaps/sky80/pz.png",
        "../envmaps/sky80/nz.png",
    };
    skybox.load(skyboxFaces);

    Shader instancedShader;
    instancedShader.load("shaders/instanced.vert", "shaders/instanced.frag");

    Mesh instancedCubes = Mesh::makeCube();
    std::vector<Mat4> instanceMatrices;
    for (int gx = -4; gx <= 4; ++gx) {
        for (int gz = -4; gz <= 4; ++gz) {
            Mat4 translation = mat4_translation((float)gx * 1.5f, 2.5f, (float)gz * 1.5f);
            Mat4 scaling = mat4_scale(0.2f, 0.2f, 0.2f);
            instanceMatrices.push_back(mat4_mul(translation, scaling));
        }
    }
    instancedCubes.setInstanceMatrices(instanceMatrices);

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
        Mat4 view = camera.view();
        Mat4 proj = camera.proj(aspect);
        cameraUBO.update(view, proj, camera.position());

        fbo.bind();
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        phong.use();
        phong.setVec3("uLightDir",    vec3_norm(scene.light.direction));
        phong.setVec3("uLightColor",  scene.light.color);
        phong.setVec3("uSkyColor",    scene.ambient.skyColor);
        phong.setVec3("uGroundColor", scene.ambient.groundColor);
        phong.setInt("uDiffuseTex", 0);

        for (size_t k = 0; k < scene.objects().size(); ++k) {
            const SceneObject& obj = scene.objects()[k];
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
            obj.mesh.draw();
        }

        instancedShader.use();
        instancedShader.setMat4("uView", view);
        instancedShader.setMat4("uProj", proj);
        instancedShader.setVec3("uColor", 0.9f, 0.8f, 0.2f);
        instancedCubes.drawInstanced();

        skybox.draw(view, proj);

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

    skybox.destroy();
    instancedShader.destroy();
    cameraUBO.destroy();
    fbo.destroy();
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
