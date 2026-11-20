#pragma once

#include "gl_api.h"
#include <glm/glm.hpp>
#include <cstddef>

class Mesh {
public:
    static constexpr int gridDivisions = 100;
    static Mesh cube();
    static Mesh guides();

    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    void bind() const;

private:
    Mesh(const void* data, std::size_t size, bool textured);
    GLuint vao = 0;
    GLuint buffer = 0;
};
