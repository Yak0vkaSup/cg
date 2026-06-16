#include "Shader.h"
#include <cstdio>
#include <fstream>
#include <sstream>

static std::string read_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        std::fprintf(stderr, "[Shader] impossible d'ouvrir %s\n", path.c_str());
        return {};
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static GLuint compile(GLenum type, const char* src, const std::string& name) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, 1024, nullptr, log);
        std::fprintf(stderr, "[Shader] erreur compilation %s:\n%s\n", name.c_str(), log);
    }
    return s;
}

bool Shader::load(const std::string& vertPath, const std::string& fragPath) {
    std::string vsrc = read_file(vertPath);
    std::string fsrc = read_file(fragPath);
    if (vsrc.empty() || fsrc.empty()) return false;

    GLuint vs = compile(GL_VERTEX_SHADER,   vsrc.c_str(), vertPath);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsrc.c_str(), fragPath);

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);

    GLint ok = 0;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(program_, 1024, nullptr, log);
        std::fprintf(stderr, "[Shader] erreur link:\n%s\n", log);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return ok != 0;
}

void Shader::destroy() {
    if (program_) glDeleteProgram(program_);
    program_ = 0;
}

void Shader::use() const { glUseProgram(program_); }

int Shader::loc(const std::string& name) const {
    return glGetUniformLocation(program_, name.c_str());
}

void Shader::setInt(const std::string& name, int v) const   { glUniform1i(loc(name), v); }
void Shader::setFloat(const std::string& name, float v) const { glUniform1f(loc(name), v); }
void Shader::setVec3(const std::string& name, Vec3 v) const  { glUniform3f(loc(name), v.x, v.y, v.z); }
void Shader::setVec3(const std::string& name, float x, float y, float z) const { glUniform3f(loc(name), x, y, z); }
void Shader::setMat4(const std::string& name, const Mat4& m) const {
    glUniformMatrix4fv(loc(name), 1, GL_FALSE, m.m);
}

void Shader::bindUniformBlock(const std::string& blockName, GLuint bindingPoint) const {
    GLuint idx = glGetUniformBlockIndex(program_, blockName.c_str());
    if (idx != GL_INVALID_INDEX)
        glUniformBlockBinding(program_, idx, bindingPoint);
}
