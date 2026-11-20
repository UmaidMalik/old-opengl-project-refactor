#include "shader.h"

#include <fstream>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <glm/gtc/type_ptr.hpp>

namespace {

std::string readShader(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not open shader: " + path.string());
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

GLuint compile(GLenum stage, const char* path) {
    constexpr const char* includePrefix = "#include \"";
    std::istringstream input(readShader(path));
    std::ostringstream expanded;
    for (std::string line; std::getline(input, line); ) {
        if (line.rfind(includePrefix, 0) == 0) {
            const auto begin = std::char_traits<char>::length(includePrefix);
            const auto end = line.find('"', begin);
            if (end == std::string::npos) throw std::runtime_error("Unclosed shader include.");
            const auto include = std::filesystem::path(path).parent_path() / line.substr(begin, end - begin);
            expanded << readShader(include) << '\n';
        } else {
            expanded << line << '\n';
        }
    }
    auto source = expanded.str();
#ifdef __EMSCRIPTEN__
    source.replace(0, source.find('\n'), "#version 300 es\nprecision highp float;\nprecision highp int;");
#endif
    const char* code = source.c_str();
    const auto shader = glCreateShader(stage);
    glShaderSource(shader, 1, &code, nullptr);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char message[2048]{};
        glGetShaderInfoLog(shader, sizeof(message), nullptr, message);
        glDeleteShader(shader);
        throw std::runtime_error(std::string(path) + "\n" + message);
    }
    return shader;
}

}  // namespace

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    const auto vertex = compile(GL_VERTEX_SHADER, vertexPath);
    GLuint fragment;
    try {
        fragment = compile(GL_FRAGMENT_SHADER, fragmentPath);
    } catch (...) {
        glDeleteShader(vertex);
        throw;
    }
    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        char message[2048]{};
        glGetProgramInfoLog(program, sizeof(message), nullptr, message);
        glDeleteProgram(program);
        throw std::runtime_error(std::string("Could not link shader program: ") + message);
    }
}

Shader::~Shader() {
    glDeleteProgram(program);
}

void Shader::useProgram() const {
    glUseProgram(program);
}

void Shader::setBool(const char* name, bool value) const {
    setInt(name, value);
}

void Shader::setInt(const char* name, int value) const {
    glUniform1i(glGetUniformLocation(program, name), value);
}

void Shader::setFloat(const char* name, float value) const {
    glUniform1f(glGetUniformLocation(program, name), value);
}

void Shader::setVec3(const char* name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(program, name), 1, glm::value_ptr(value));
}

void Shader::setMat4(const char* name, const glm::mat4& value) const {
    glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(value));
}
