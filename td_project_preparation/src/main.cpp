#include <glad/gl.h>
#include <SDL.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include "mat4.h"

static std::string read_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        SDL_Log("cannot open %s", path.c_str());
        return {};
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static GLuint compile_shader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, 1024, nullptr, log);
        SDL_Log("shader error: %s", log);
    }
    return s;
}

static GLuint make_program(const std::string& vertPath, const std::string& fragPath) {
    std::string vsrc = read_file(vertPath);
    std::string fsrc = read_file(fragPath);
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vsrc.c_str());
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fsrc.c_str());
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(p, 1024, nullptr, log);
        SDL_Log("link error: %s", log);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}

// cube 36 vertices position + normale
static const float CUBE[] = {
    // back face (-Z)
    -0.5f,-0.5f,-0.5f,  0,0,-1,
     0.5f, 0.5f,-0.5f,  0,0,-1,
     0.5f,-0.5f,-0.5f,  0,0,-1,
     0.5f, 0.5f,-0.5f,  0,0,-1,
    -0.5f,-0.5f,-0.5f,  0,0,-1,
    -0.5f, 0.5f,-0.5f,  0,0,-1,
    // front face (+Z)
    -0.5f,-0.5f, 0.5f,  0,0,1,
     0.5f,-0.5f, 0.5f,  0,0,1,
     0.5f, 0.5f, 0.5f,  0,0,1,
     0.5f, 0.5f, 0.5f,  0,0,1,
    -0.5f, 0.5f, 0.5f,  0,0,1,
    -0.5f,-0.5f, 0.5f,  0,0,1,
    // left face (-X)
    -0.5f, 0.5f, 0.5f, -1,0,0,
    -0.5f, 0.5f,-0.5f, -1,0,0,
    -0.5f,-0.5f,-0.5f, -1,0,0,
    -0.5f,-0.5f,-0.5f, -1,0,0,
    -0.5f,-0.5f, 0.5f, -1,0,0,
    -0.5f, 0.5f, 0.5f, -1,0,0,
    // right face (+X)
     0.5f, 0.5f, 0.5f,  1,0,0,
     0.5f,-0.5f,-0.5f,  1,0,0,
     0.5f, 0.5f,-0.5f,  1,0,0,
     0.5f,-0.5f,-0.5f,  1,0,0,
     0.5f, 0.5f, 0.5f,  1,0,0,
     0.5f,-0.5f, 0.5f,  1,0,0,
    // bottom (-Y)
    -0.5f,-0.5f,-0.5f,  0,-1,0,
     0.5f,-0.5f,-0.5f,  0,-1,0,
     0.5f,-0.5f, 0.5f,  0,-1,0,
     0.5f,-0.5f, 0.5f,  0,-1,0,
    -0.5f,-0.5f, 0.5f,  0,-1,0,
    -0.5f,-0.5f,-0.5f,  0,-1,0,
    // top (+Y)
    -0.5f, 0.5f,-0.5f,  0,1,0,
     0.5f, 0.5f, 0.5f,  0,1,0,
     0.5f, 0.5f,-0.5f,  0,1,0,
     0.5f, 0.5f, 0.5f,  0,1,0,
    -0.5f, 0.5f,-0.5f,  0,1,0,
    -0.5f, 0.5f, 0.5f,  0,1,0,
};

struct OrbitCam {
    float radius = 5.0f;
    float phi = 0.0f;     // azimut
    float theta = 0.3f;   // elevation
};

static Vec3 orbit_position(const OrbitCam& c, Vec3 target) {
    float x = c.radius * std::cos(c.theta) * std::cos(c.phi);
    float y = c.radius * std::sin(c.theta);
    float z = c.radius * std::cos(c.theta) * std::sin(c.phi);
    return { target.x + x, target.y + y, target.z + z };
}

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    int W = 1024, H = 768;
    SDL_Window* win = SDL_CreateWindow(
        "Camera orbitale",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        W, H,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    if (!win) { SDL_Log("CreateWindow: %s", SDL_GetError()); return 1; }

    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    if (!ctx) { SDL_Log("GL_CreateContext: %s", SDL_GetError()); return 1; }
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        SDL_Log("gladLoadGL failed");
        return 1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    char* base = SDL_GetBasePath();
    std::string baseDir = base ? std::string(base) : std::string();
    SDL_free(base);
    GLuint prog = make_program(baseDir + "shaders/basic.vert", baseDir + "shaders/basic.frag");

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE), CUBE, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    GLint locWorld = glGetUniformLocation(prog, "uWorld");
    GLint locView  = glGetUniformLocation(prog, "uView");
    GLint locProj  = glGetUniformLocation(prog, "uProj");
    GLint locColor = glGetUniformLocation(prog, "uColor");

    OrbitCam cam;
    Vec3 target = { 0, 0, 0 };
    bool dragging = false;

    const float PI = 3.14f;

    bool running = true;
    Uint32 last = SDL_GetTicks();
    float t = 0.0f;

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
            else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                W = e.window.data1; H = e.window.data2;
                glViewport(0, 0, W, H);
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                dragging = true;
            }
            else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                dragging = false;
            }
            else if (e.type == SDL_MOUSEMOTION && dragging) {
                cam.phi   += e.motion.xrel * 0.005f;
                cam.theta += e.motion.yrel * 0.005f;
                if (cam.phi > PI) cam.phi -= 2*PI;
                if (cam.phi < -PI) cam.phi += 2*PI;
                float lim = PI*0.5f - 0.01f;
                if (cam.theta > lim) cam.theta = lim;
                if (cam.theta < -lim) cam.theta = -lim;
            }
            else if (e.type == SDL_MOUSEWHEEL) {
                cam.radius -= e.wheel.y * 0.5f;
                if (cam.radius < 1.0f) cam.radius = 1.0f;
                if (cam.radius > 50.0f) cam.radius = 50.0f;
            }
        }

        Uint32 now = SDL_GetTicks();
        float dt = (now - last) / 1000.0f;
        last = now;
        t += dt;

        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Vec3 camPos = orbit_position(cam, target);
        Mat4 view = mat4_lookAt(camPos, target, { 0, 1, 0 });
        Mat4 proj = mat4_perspective(60.0f * PI / 180.0f, (float)W / (float)H, 0.1f, 100.0f);

        glUseProgram(prog);
        glUniformMatrix4fv(locView, 1, GL_FALSE, view.m);
        glUniformMatrix4fv(locProj, 1, GL_FALSE, proj.m);

        glBindVertexArray(vao);

        // cube1 centrre tourne
        {
            Mat4 T = mat4_translation(0, 0, 0);
            Mat4 R = mat4_mul(mat4_rotY(t * 0.7f), mat4_rotX(t * 0.3f));
            Mat4 S = mat4_scale(1.0f, 1.0f, 1.0f);
            Mat4 W = mat4_mul(T, mat4_mul(R, S));
            glUniformMatrix4fv(locWorld, 1, GL_FALSE, W.m);
            glUniform3f(locColor, 0.9f, 0.4f, 0.3f);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // cube2 sur le cote plus petit
        {
            Mat4 T = mat4_translation(2.0f, 0.5f, -1.0f);
            Mat4 R = mat4_rotY(t * -0.5f);
            Mat4 S = mat4_scale(0.6f, 1.2f, 0.6f);
            Mat4 W = mat4_mul(T, mat4_mul(R, S));
            glUniformMatrix4fv(locWorld, 1, GL_FALSE, W.m);
            glUniform3f(locColor, 0.3f, 0.7f, 0.9f);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // sol
        {
            Mat4 T = mat4_translation(0, -1.0f, 0);
            Mat4 S = mat4_scale(10.0f, 0.05f, 10.0f);
            Mat4 W = mat4_mul(T, S);
            glUniformMatrix4fv(locWorld, 1, GL_FALSE, W.m);
            glUniform3f(locColor, 0.5f, 0.5f, 0.55f);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        SDL_GL_SwapWindow(win);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
