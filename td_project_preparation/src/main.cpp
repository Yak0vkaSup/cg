#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include "mat4.h"

static std::string read_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        std::fprintf(stderr, "cannot open %s\n", path.c_str());
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
        std::fprintf(stderr, "shader error: %s\n", log);
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
        std::fprintf(stderr, "link error: %s\n", log);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}

// cube 36 vertices position + normale
static const float CUBE[] = {
    -0.5f,-0.5f,-0.5f,  0,0,-1,
     0.5f, 0.5f,-0.5f,  0,0,-1,
     0.5f,-0.5f,-0.5f,  0,0,-1,
     0.5f, 0.5f,-0.5f,  0,0,-1,
    -0.5f,-0.5f,-0.5f,  0,0,-1,
    -0.5f, 0.5f,-0.5f,  0,0,-1,

    -0.5f,-0.5f, 0.5f,  0,0,1,
     0.5f,-0.5f, 0.5f,  0,0,1,
     0.5f, 0.5f, 0.5f,  0,0,1,
     0.5f, 0.5f, 0.5f,  0,0,1,
    -0.5f, 0.5f, 0.5f,  0,0,1,
    -0.5f,-0.5f, 0.5f,  0,0,1,

    -0.5f, 0.5f, 0.5f, -1,0,0,
    -0.5f, 0.5f,-0.5f, -1,0,0,
    -0.5f,-0.5f,-0.5f, -1,0,0,
    -0.5f,-0.5f,-0.5f, -1,0,0,
    -0.5f,-0.5f, 0.5f, -1,0,0,
    -0.5f, 0.5f, 0.5f, -1,0,0,

     0.5f, 0.5f, 0.5f,  1,0,0,
     0.5f,-0.5f,-0.5f,  1,0,0,
     0.5f, 0.5f,-0.5f,  1,0,0,
     0.5f,-0.5f,-0.5f,  1,0,0,
     0.5f, 0.5f, 0.5f,  1,0,0,
     0.5f,-0.5f, 0.5f,  1,0,0,

    -0.5f,-0.5f,-0.5f,  0,-1,0,
     0.5f,-0.5f,-0.5f,  0,-1,0,
     0.5f,-0.5f, 0.5f,  0,-1,0,
     0.5f,-0.5f, 0.5f,  0,-1,0,
    -0.5f,-0.5f, 0.5f,  0,-1,0,
    -0.5f,-0.5f,-0.5f,  0,-1,0,

    -0.5f, 0.5f,-0.5f,  0,1,0,
     0.5f, 0.5f, 0.5f,  0,1,0,
     0.5f, 0.5f,-0.5f,  0,1,0,
     0.5f, 0.5f, 0.5f,  0,1,0,
    -0.5f, 0.5f,-0.5f,  0,1,0,
    -0.5f, 0.5f, 0.5f,  0,1,0,
};

struct OrbitCam {
    float radius = 5.0f;
    float phi = 0.0f;
    float theta = 0.3f;
};

struct AppState {
    OrbitCam cam;
    bool dragging = false;
    double lastX = 0, lastY = 0;
    int W = 1024, H = 768;
};

static const float PI = 3.14159265358979f;

static Vec3 orbit_position(const OrbitCam& c, Vec3 target) {
    float x = c.radius * std::cos(c.theta) * std::cos(c.phi);
    float y = c.radius * std::sin(c.theta);
    float z = c.radius * std::cos(c.theta) * std::sin(c.phi);
    return { target.x + x, target.y + y, target.z + z };
}

static void on_resize(GLFWwindow* w, int width, int height) {
    auto* s = (AppState*)glfwGetWindowUserPointer(w);
    s->W = width; s->H = height;
    glViewport(0, 0, width, height);
}

static void on_mouse_button(GLFWwindow* w, int button, int action, int mods) {
    auto* s = (AppState*)glfwGetWindowUserPointer(w);
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            s->dragging = true;
            glfwGetCursorPos(w, &s->lastX, &s->lastY);
        } else if (action == GLFW_RELEASE) {
            s->dragging = false;
        }
    }
}

static void on_cursor_pos(GLFWwindow* w, double x, double y) {
    auto* s = (AppState*)glfwGetWindowUserPointer(w);
    if (!s->dragging) return;
    double dx = x - s->lastX;
    double dy = y - s->lastY;
    s->lastX = x; s->lastY = y;
    s->cam.phi   += (float)dx * 0.005f;
    s->cam.theta += (float)dy * 0.005f;
    if (s->cam.phi > PI) s->cam.phi -= 2*PI;
    if (s->cam.phi < -PI) s->cam.phi += 2*PI;
    float lim = PI*0.5f - 0.01f;
    if (s->cam.theta > lim) s->cam.theta = lim;
    if (s->cam.theta < -lim) s->cam.theta = -lim;
}

static void on_scroll(GLFWwindow* w, double xoff, double yoff) {
    auto* s = (AppState*)glfwGetWindowUserPointer(w);
    s->cam.radius -= (float)yoff * 0.5f;
    if (s->cam.radius < 1.0f) s->cam.radius = 1.0f;
    if (s->cam.radius > 50.0f) s->cam.radius = 50.0f;
}

static void on_key(GLFWwindow* w, int key, int sc, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(w, GLFW_TRUE);
    }
}

int main(int argc, char** argv) {
    if (!glfwInit()) {
        std::fprintf(stderr, "glfwInit failed\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    AppState state;
    GLFWwindow* win = glfwCreateWindow(state.W, state.H, "Camera orbitale", nullptr, nullptr);
    if (!win) {
        std::fprintf(stderr, "CreateWindow failed\n");
        glfwTerminate();
        return 1;
    }
    glfwSetWindowUserPointer(win, &state);
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

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    std::filesystem::path exeDir = std::filesystem::path(argv[0]).parent_path();
    std::string vertPath = (exeDir / "shaders" / "basic.vert").string();
    std::string fragPath = (exeDir / "shaders" / "basic.frag").string();
    GLuint prog = make_program(vertPath, fragPath);

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

    Vec3 target = { 0, 0, 0 };

    double last = glfwGetTime();
    float t = 0.0f;

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        double now = glfwGetTime();
        float dt = (float)(now - last);
        last = now;
        t += dt;

        glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Vec3 camPos = orbit_position(state.cam, target);
        Mat4 view = mat4_lookAt(camPos, target, { 0, 1, 0 });
        Mat4 proj = mat4_perspective(60.0f * PI / 180.0f, (float)state.W / (float)state.H, 0.1f, 100.0f);

        glUseProgram(prog);
        glUniformMatrix4fv(locView, 1, GL_FALSE, view.m);
        glUniformMatrix4fv(locProj, 1, GL_FALSE, proj.m);

        glBindVertexArray(vao);

        // cube 1
        {
            Mat4 T = mat4_translation(0, 0, 0);
            Mat4 R = mat4_mul(mat4_rotY(t * 0.7f), mat4_rotX(t * 0.3f));
            Mat4 S = mat4_scale(1.0f, 1.0f, 1.0f);
            Mat4 W = mat4_mul(T, mat4_mul(R, S));
            glUniformMatrix4fv(locWorld, 1, GL_FALSE, W.m);
            glUniform3f(locColor, 0.9f, 0.4f, 0.3f);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // cube 2
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

        glfwSwapBuffers(win);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
