#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include "camera.h"
#include "gl_utils.h"
#include "math3d.h"
#include "mesh.h"

namespace fs = std::filesystem;

struct CameraUBO {
  float view[16];
  float proj[16];
  float camPos[4];
};

struct ObjectUBO {
  float model[16];
  float normalMat[16];
};

struct SceneObject {
  Mesh *mesh = nullptr;
  Vec3 position{0, 0, 0};
  Vec3 scale{1, 1, 1};
  float yaw = 0.0f; // rotation fixe autour de Y
  GLuint overrideTex = 0;
};

struct App {
  OrbitCamera cam;
  bool dragging = false;
  double lastX = 0, lastY = 0;
  int W = 1280, H = 800;

  bool blinn = true;
  bool envMap = true;
  bool hemiAmbient = true;
  bool rim = true;
  bool schlick = true;
  bool drawSkybox = true;
  bool drawInstances = true;
  float rimPower = 3.0f;
  int postMode = 0;
  int instanceCount = 0;
};

static void on_resize(GLFWwindow *w, int width, int height) {
  auto *a = (App *)glfwGetWindowUserPointer(w);
  a->W = width > 1 ? width : 1;
  a->H = height > 1 ? height : 1;
}
static void on_mouse_button(GLFWwindow *w, int button, int action, int mods) {
  auto *a = (App *)glfwGetWindowUserPointer(w);
  if (ImGui::GetIO().WantCaptureMouse)
    return;
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      a->dragging = true;
      glfwGetCursorPos(w, &a->lastX, &a->lastY);
    } else if (action == GLFW_RELEASE) {
      a->dragging = false;
    }
  }
}
static void on_cursor_pos(GLFWwindow *w, double x, double y) {
  auto *a = (App *)glfwGetWindowUserPointer(w);
  if (!a->dragging)
    return;
  double dx = x - a->lastX, dy = y - a->lastY;
  a->lastX = x;
  a->lastY = y;
  a->cam.rotate((float)dx * 0.005f, (float)dy * 0.005f);
}
static void on_scroll(GLFWwindow *w, double xoff, double yoff) {
  auto *a = (App *)glfwGetWindowUserPointer(w);
  if (ImGui::GetIO().WantCaptureMouse)
    return;
  a->cam.zoom((float)yoff * 0.6f);
}
static void on_key(GLFWwindow *w, int key, int sc, int action, int mods) {
  auto *a = (App *)glfwGetWindowUserPointer(w);
  if (action != GLFW_PRESS)
    return;
  if (key == GLFW_KEY_ESCAPE)
    glfwSetWindowShouldClose(w, GLFW_TRUE);
  if (key == GLFW_KEY_R)
    a->cam = OrbitCamera{};
}

static const float SKYBOX_VERTS[] = {
    -1, -1, -1, -1, -1, 1,  -1, 1,  1,  -1, 1,  1,  -1, 1,  -1, -1, -1, -1,
    1,  -1, -1, 1,  1,  -1, 1,  1,  1,  1,  1,  1,  1,  -1, 1,  1,  -1, -1,
    -1, -1, -1, -1, 1,  -1, 1,  1,  -1, 1,  1,  -1, 1,  -1, -1, -1, -1, -1,
    -1, -1, 1,  1,  -1, 1,  1,  1,  1,  1,  1,  1,  -1, 1,  1,  -1, -1, 1,
    -1, 1,  -1, -1, 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  -1, -1, 1,  -1,
    -1, -1, -1, 1,  -1, -1, 1,  -1, 1,  1,  -1, 1,  -1, -1, 1,  -1, -1, -1,
};

static const float QUAD_VERTS[] = {
    -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
};

int main(int argc, char **argv) {
  if (!glfwInit()) {
    std::fprintf(stderr, "glfwInit failed\n");
    return 1;
  }

  auto setGLHints = [](int major, int minor) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  };

  App app;
  // on tente avec 4.3 (compute shader)  sinon retour  sur 4.1 (MacOS)
  setGLHints(4, 3);
  GLFWwindow *win = glfwCreateWindow(
      app.W, app.H, "Projet OpenGL M1 - Scene 3D", nullptr, nullptr);
  if (!win) {
    setGLHints(4, 1);
    win = glfwCreateWindow(app.W, app.H, "Projet OpenGL M1 - Scene 3D", nullptr,
                           nullptr);
  }
  if (!win) {
    std::fprintf(stderr, "CreateWindow failed (need OpenGL 4.1)\n");
    glfwTerminate();
    return 1;
  }
  glfwSetWindowUserPointer(win, &app);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);

  glfwSetFramebufferSizeCallback(win, on_resize);
  glfwSetMouseButtonCallback(win, on_mouse_button);
  glfwSetCursorPosCallback(win, on_cursor_pos);
  glfwSetScrollCallback(win, on_scroll);
  glfwSetKeyCallback(win, on_key);

  if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
    std::fprintf(stderr, "gladLoadGL failed\n");
    return 1;
  }
  std::printf("OpenGL %s\n", glGetString(GL_VERSION));
  // compute shader disponible seulement si le contexte est >= 4.3
  bool hasCompute = (GLAD_GL_VERSION_4_3 != 0);
  std::printf("Compute shader : %s\n",
              hasCompute ? "disponible (GL 4.3)" : "indisponible (fallback)");
  glfwGetFramebufferSize(win, &app.W, &app.H);

  glEnable(GL_DEPTH_TEST);

  fs::path exeDir = fs::path(argv[0]).parent_path();
  std::string shaderDir = (exeDir / "shaders").string();
  std::string assetDir = (exeDir / "assets").string();
  auto sp = [&](const char *f) { return (fs::path(shaderDir) / f).string(); };

  GLuint progPhong = glu::program_vf(sp("phong.vert"), sp("phong.frag"));
  GLuint progSkybox = glu::program_vf(sp("skybox.vert"), sp("skybox.frag"));
  GLuint progPost = glu::program_vf(sp("post.vert"), sp("post.frag"));
  GLuint progInstanced =
      glu::program_vf(sp("instanced.vert"), sp("instanced.frag"));
  GLuint progProcedural =
      glu::program_vf(sp("post.vert"), sp("procedural.frag"));
  GLuint progProceduralCompute =
      hasCompute ? glu::program_compute(sp("procedural.comp")) : 0;

  auto bindBlock = [](GLuint prog, const char *name, GLuint binding) {
    GLuint idx = glGetUniformBlockIndex(prog, name);
    if (idx != GL_INVALID_INDEX)
      glUniformBlockBinding(prog, idx, binding);
  };
  bindBlock(progPhong, "Camera", 0);
  bindBlock(progPhong, "Object", 1);
  bindBlock(progSkybox, "Camera", 0);
  bindBlock(progInstanced, "Camera", 0);

  glUseProgram(progPhong);
  glUniform1i(glGetUniformLocation(progPhong, "uDiffuseTex"), 0);
  glUniform1i(glGetUniformLocation(progPhong, "uEnvMap"), 1);
  glUseProgram(progInstanced);
  glUniform1i(glGetUniformLocation(progInstanced, "uEnvMap"), 1);
  glUseProgram(progSkybox);
  glUniform1i(glGetUniformLocation(progSkybox, "uEnvMap"), 1);
  glUseProgram(progPost);
  glUniform1i(glGetUniformLocation(progPost, "uScene"), 0);

  // petits helpers just pour se simplifier la life
  auto setBool = [](GLuint p, const char *n, bool v) {
    glUniform1i(glGetUniformLocation(p, n), v ? 1 : 0);
  };
  auto setFloat = [](GLuint p, const char *n, float v) {
    glUniform1f(glGetUniformLocation(p, n), v);
  };
  auto setVec3 = [](GLuint p, const char *n, Vec3 v) {
    glUniform3f(glGetUniformLocation(p, n), v.x, v.y, v.z);
  };

  GLuint cameraUBO, objectUBO;
  glGenBuffers(1, &cameraUBO);
  glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraUBO), nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);

  glGenBuffers(1, &objectUBO);
  glBindBuffer(GL_UNIFORM_BUFFER, objectUBO);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(ObjectUBO), nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, objectUBO);

  GLuint envCube = glu::load_cubemap(assetDir, ".png", true);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_CUBE_MAP, envCube);

  // VAO/VBO de la skybox
  GLuint skyVao, skyVbo;
  glGenVertexArrays(1, &skyVao);
  glGenBuffers(1, &skyVbo);
  glBindVertexArray(skyVao);
  glBindBuffer(GL_ARRAY_BUFFER, skyVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(SKYBOX_VERTS), SKYBOX_VERTS,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // VAO/VBO du quad
  GLuint quadVao, quadVbo;
  glGenVertexArrays(1, &quadVao);
  glGenBuffers(1, &quadVbo);
  glBindVertexArray(quadVao);
  glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTS), QUAD_VERTS, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glBindVertexArray(0);

  // texture procedurale
  const int PROC = 512;
  GLuint procTex;
  glGenTextures(1, &procTex);
  glBindTexture(GL_TEXTURE_2D, procTex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, PROC, PROC, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  GLuint procFbo;
  glGenFramebuffers(1, &procFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, procFbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         procTex, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // chargement des modeles
  Mesh mPlane, mSphere, mTorus, mCube, mSphereBlue, mProcSphere, mInstance;
  mPlane.load((fs::path(assetDir) / "plane.obj").string(), assetDir);
  mSphere.load((fs::path(assetDir) / "sphere.obj").string(), assetDir);
  mTorus.load((fs::path(assetDir) / "torus.obj").string(), assetDir);
  mCube.load((fs::path(assetDir) / "cube.obj").string(), assetDir);
  mSphereBlue.load((fs::path(assetDir) / "sphere_blue.obj").string(), assetDir);
  mProcSphere.load((fs::path(assetDir) / "sphere.obj").string(), assetDir);
  mInstance.load((fs::path(assetDir) / "cube.obj").string(), assetDir);

  // matrice monde + couleur pour chaque cube
  std::vector<float> inst;
  auto pushInstance = [&](Mat4 m, Vec3 c) {
    for (int i = 0; i < 16; ++i)
      inst.push_back(m.m[i]);
    inst.push_back(c.x);
    inst.push_back(c.y);
    inst.push_back(c.z);
    inst.push_back(1.0f);
  };
  {
    const Vec3 palette[5] = {{0.90f, 0.30f, 0.30f},
                             {0.30f, 0.80f, 0.45f},
                             {0.35f, 0.55f, 0.95f},
                             {0.95f, 0.80f, 0.30f},
                             {0.75f, 0.40f, 0.90f}};
    int idx = 0;
    for (int ring = 0; ring < 2; ++ring) {
      float R = 7.0f + ring * 1.6f;
      int n = 30 + ring * 6;
      for (int i = 0; i < n; ++i) {
        float a = (float)i / n * 2.0f * PI;
        Vec3 p{R * std::cos(a), 0.35f + ring * 0.5f, R * std::sin(a)};
        Mat4 m =
            mat4_mul(mat4_translation(p.x, p.y, p.z),
                     mat4_mul(mat4_rotY(a), mat4_scale(0.28f, 0.28f, 0.28f)));
        pushInstance(m, palette[idx % 5]);
        ++idx;
      }
    }
  }
  app.instanceCount = (int)(inst.size() / 20);
  mInstance.setup_instancing(inst, 20);

  // on place les objects
  std::vector<SceneObject> scene = {
      {&mPlane, {0.0f, 0.0f, 0.0f}, {1, 1, 1}, 0.0f, 0},
      {&mSphere, {-2.6f, 1.0f, 0.0f}, {1, 1, 1}, 0.0f, 0},
      {&mTorus, {2.6f, 1.3f, 0.0f}, {1, 1, 1}, 1.0f, 0},
      {&mSphereBlue, {0.0f, 1.0f, 2.8f}, {0.9f, 0.9f, 0.9f}, 0.0f, 0},
      {&mCube, {0.0f, 0.7f, -2.8f}, {1, 1, 1}, 0.6f, 0},
      {&mProcSphere, {0.0f, 2.6f, 0.0f}, {0.8f, 0.8f, 0.8f}, 0.0f, procTex},
  };

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(win, true);
  ImGui_ImplOpenGL3_Init("#version 150");

  glu::Framebuffer scene_fbo;
  scene_fbo.create(app.W, app.H);

  // texture procedurale
  if (hasCompute) {
    // compute shader
    glUseProgram(progProceduralCompute);
    glBindImageTexture(0, procTex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glDispatchCompute(PROC / 16, PROC / 16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                    GL_TEXTURE_FETCH_BARRIER_BIT);
  } else {
    // fallback (macOS 4.1) -> render-to-texture via un FBO
    glBindFramebuffer(GL_FRAMEBUFFER, procFbo);
    glViewport(0, 0, PROC, PROC);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(progProcedural);
    glBindVertexArray(quadVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  CameraUBO cu{};
  ObjectUBO ou{};

  while (!glfwWindowShouldClose(win)) {
    glfwPollEvents();

    scene_fbo.resize(app.W, app.H);

    // rendu de la scene dans le FBO
    glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo.fbo);
    glViewport(0, 0, app.W, app.H);
    glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // matrices camera -> UBO
    Vec3 eye = app.cam.position();
    Mat4 view = app.cam.view();
    Mat4 proj = mat4_perspective(60.0f * PI / 180.0f,
                                 (float)app.W / (float)app.H, 0.1f, 200.0f);
    std::memcpy(cu.view, view.m, sizeof(view.m));
    std::memcpy(cu.proj, proj.m, sizeof(proj.m));
    cu.camPos[0] = eye.x;
    cu.camPos[1] = eye.y;
    cu.camPos[2] = eye.z;
    cu.camPos[3] = 1.0f;
    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraUBO), &cu);

    // lumieres
    Vec3 keyDir = vec3_norm(Vec3{0.4f, 0.95f, 0.5f});
    Vec3 fillDir = vec3_norm(Vec3{-0.5f, 0.6f, -0.4f});

    glUseProgram(progPhong);
    setBool(progPhong, "uUseBlinn", app.blinn);
    setBool(progPhong, "uUseSchlick", app.schlick);
    setBool(progPhong, "uUseHemi", app.hemiAmbient);
    setBool(progPhong, "uUseEnv", app.envMap);
    setBool(progPhong, "uUseRim", app.rim);
    setFloat(progPhong, "uRimPower", app.rimPower);
    glUniform1i(glGetUniformLocation(progPhong, "uNumLights"), 2);
    setVec3(progPhong, "uLights[0].direction", keyDir);
    setVec3(progPhong, "uLights[0].color", Vec3{0.90f, 0.88f, 0.84f});
    setVec3(progPhong, "uLights[1].direction", fillDir);
    setVec3(progPhong, "uLights[1].color", Vec3{0.22f, 0.17f, 0.14f});

    for (const SceneObject &o : scene) {
      if (!o.mesh)
        continue;
      Mat4 model =
          mat4_mul(mat4_translation(o.position.x, o.position.y, o.position.z),
                   mat4_mul(mat4_rotY(o.yaw),
                            mat4_scale(o.scale.x, o.scale.y, o.scale.z)));
      Mat4 nrm = mat4_normal_matrix(model);
      std::memcpy(ou.model, model.m, sizeof(model.m));
      std::memcpy(ou.normalMat, nrm.m, sizeof(nrm.m));
      glBindBuffer(GL_UNIFORM_BUFFER, objectUBO);
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ObjectUBO), &ou);

      o.mesh->draw([&](const Material &mat) {
        Vec3 kd = mat.Kd;
        GLuint tex;
        bool hasTex;
        if (o.overrideTex) {
          tex = o.overrideTex;
          hasTex = true;
          kd = Vec3{1.0f, 1.0f, 1.0f};
        } else if (mat.hasTexture) {
          tex = mat.diffuseTex;
          hasTex = true;
        } else {
          tex = 0;
          hasTex = false;
        }
        setVec3(progPhong, "uMaterial.Ka", mat.Ka);
        setVec3(progPhong, "uMaterial.Kd", kd);
        setVec3(progPhong, "uMaterial.Ks", mat.Ks);
        setFloat(progPhong, "uMaterial.Ns", mat.Ns);
        setBool(progPhong, "uHasTexture", hasTex);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hasTex ? tex : 0);
      });
    }

    // anneau de cubes
    if (app.drawInstances) {
      glUseProgram(progInstanced);
      setVec3(progInstanced, "uLightDir", keyDir);
      setVec3(progInstanced, "uLightColor", Vec3{0.80f, 0.80f, 0.80f});
      setBool(progInstanced, "uUseEnv", app.envMap);
      mInstance.draw_instanced(app.instanceCount);
    }

    // skybox en dernier
    if (app.drawSkybox) {
      glDepthFunc(GL_LEQUAL);
      glUseProgram(progSkybox);
      glBindVertexArray(skyVao);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      glDepthFunc(GL_LESS);
    }

    // post-traitement  FBO -> ecran
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, app.W, app.H);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(progPost);
    glUniform1i(glGetUniformLocation(progPost, "uPostMode"), app.postMode);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene_fbo.color);
    glBindVertexArray(quadVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);

    // interface
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_FirstUseEver);
    ImGui::Begin("Projet OpenGL M1");
    ImGui::Text("%.1f FPS  |  OpenGL 4.1 core", ImGui::GetIO().Framerate);
    ImGui::Text("drag: orbit   wheel: zoom   R: reset");
    ImGui::SeparatorText("Illumination");
    ImGui::Checkbox("Blinn-Phong (sinon Phong)", &app.blinn);
    ImGui::Checkbox("Fresnel de Schlick", &app.schlick);
    ImGui::Checkbox("Ambiante hemispherique", &app.hemiAmbient);
    ImGui::Checkbox("Environment mapping", &app.envMap);
    ImGui::Checkbox("Rim / back-light", &app.rim);
    ImGui::SliderFloat("Rim power", &app.rimPower, 0.5f, 8.0f);
    ImGui::SeparatorText("Scene");
    ImGui::Checkbox("Skybox cubemap", &app.drawSkybox);
    ImGui::Checkbox("Instancing", &app.drawInstances);
    ImGui::Text("instances: %d", app.instanceCount);
    ImGui::SeparatorText("Post-traitement");
    ImGui::Combo("Effet", &app.postMode, "Aucun\0Noir et blanc\0Sepia\0");
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(win);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  scene_fbo.destroy();
  glfwDestroyWindow(win);
  glfwTerminate();
  return 0;
}