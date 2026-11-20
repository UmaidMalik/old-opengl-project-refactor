#include "mesh.h"

#include <vector>

using glm::vec2;
using glm::vec3;

namespace {

struct Vertex {
    vec3 position;
    vec2 uv;
    vec3 normal;
};

// Face order: -Z, +X, +Z, -X, -Y, +Y.
const Vertex cubeVertices[] = {

    {vec3(0.0f, 0.0f, 0.0f),    vec2(0.0f, 0.0f),    vec3(0.0f, 0.0f, -1.0f)},
    {vec3(0.1f, 0.0f, 0.0f),    vec2(-1.0f, 0.0f),    vec3(0.0f, 0.0f, -1.0f)},
    {vec3(0.1f, 0.1f, 0.0f),    vec2(-1.0f, -1.0f),    vec3(0.0f, 0.0f, -1.0f)},
    {vec3(0.0f, 0.0f, 0.0f),    vec2(0.0f, 0.0f),    vec3(0.0f, 0.0f, -1.0f)},
    {vec3(0.1f, 0.1f, 0.0f),    vec2(-1.0f, -1.0f),    vec3(0.0f, 0.0f, -1.0f)},
    {vec3(0.0f, 0.1f, 0.0f),    vec2(0.0f, -1.0f),    vec3(0.0f, 0.0f, -1.0f)},

    {vec3(0.1f, 0.0f, 0.0f),    vec2(1.0f, 0.0f),    vec3(1.0f, 0.0f, 0.0f)},
    {vec3(0.1f, 0.0f, 0.1f),    vec2(0.0f, 0.0f),    vec3(1.0f, 0.0f, 0.0f)},
    {vec3(0.1f, 0.1f, 0.0f),    vec2(1.0f, -1.0f),    vec3(1.0f, 0.0f, 0.0f)},
    {vec3(0.1f, 0.1f, 0.0f),    vec2(1.0f, -1.0f),    vec3(1.0f, 0.0f, 0.0f)},
    {vec3(0.1f, 0.0f, 0.1f),    vec2(0.0f, 0.0f),  vec3(1.0f, 0.0f, 0.0f)},
    {vec3(0.1f, 0.1f, 0.1f),    vec2(0.0f, -1.0f),   vec3(1.0f, 0.0f, 0.0f)},

    {vec3(0.0f, 0.0f, 0.1f),    vec2(0.0f, 0.0f),    vec3(0.0f, 0.0f, 1.0f)},
    {vec3(0.1f, 0.1f, 0.1f),    vec2(1.0f, -1.0f),    vec3(0.0f, 0.0f, 1.0f)},
    {vec3(0.1f, 0.0f, 0.1f),    vec2(1.0f, 0.0f),    vec3(0.0f, 0.0f, 1.0f)},
    {vec3(0.0f, 0.0f, 0.1f),    vec2(0.0f, 0.0f),  vec3(0.0f, 0.0f, 1.0f)},
    {vec3(0.0f, 0.1f, 0.1f),    vec2(0.0f, -1.0f),   vec3(0.0f, 0.0f, 1.0f)},
    {vec3(0.1f, 0.1f, 0.1f),    vec2(1.0f, -1.0f),    vec3(0.0f, 0.0f, 1.0f)},

    {vec3(0.0f, 0.0f, 0.0f),    vec2(0.0f, 1.0f),   vec3(-1.0f, 0.0f, 0.0f)},
    {vec3(0.0f, 0.1f, 0.0f),    vec2(0.0f, 0.0f),    vec3(-1.0f, 0.0f, 0.0f)},
    {vec3(0.0f, 0.0f, 0.1f),    vec2(1.0f, 1.0f),  vec3(-1.0f, 0.0f, 0.0f)},
    {vec3(0.0f, 0.1f, 0.0f),    vec2(0.0f, 0.0f),    vec3(-1.0f, 0.0f, 0.0f)},
    {vec3(0.0f, 0.1f, 0.1f),    vec2(1.0f, 0.0f),   vec3(-1.0f, 0.0f, 0.0f)},
    {vec3(0.0f, 0.0f, 0.1f),    vec2(1.0f, 1.0f),  vec3(-1.0f, 0.0f, 0.0f)},

    {vec3(0.0f, 0.0f, 0.0f),    vec2(-1.0f, 1.0f),    vec3(0.0f, -1.0f, 0.0f)},
    {vec3(0.1f, 0.0f, 0.1f),    vec2(0.0f, 0.0f),    vec3(0.0f, -1.0f, 0.0f)},
    {vec3(0.1f, 0.0f, 0.0f),    vec2(0.0f, 1.0f),    vec3(0.0f, -1.0f, 0.0f)},
    {vec3(0.0f, 0.0f, 0.0f),    vec2(0.0f, 0.0f),    vec3(0.0f, -1.0f, 0.0f)},
    {vec3(0.0f, 0.0f, 0.1f),    vec2(0.0f, -1.0f),   vec3(0.0f, -1.0f, 0.0f)},
    {vec3(0.1f, 0.0f, 0.1f),    vec2(1.0f, -1.0f),  vec3(0.0f, -1.0f, 0.0f)},

    {vec3(0.0f, 0.1f, 0.0f),    vec2(-1.0f, -1.0f),    vec3(0.0f, 1.0f, 0.0f)},
    {vec3(0.1f, 0.1f, 0.0f),    vec2(0.0f, -1.0f),    vec3(0.0f, 1.0f, 0.0f)},
    {vec3(0.1f, 0.1f, 0.1f),    vec2(0.0f, 0.0f),    vec3(0.0f, 1.0f, 0.0f)},
    {vec3(0.0f, 0.1f, 0.0f),    vec2(-1.0f, -1.0f),    vec3(0.0f, 1.0f, 0.0f)},
    {vec3(0.1f, 0.1f, 0.1f),    vec2(0.0f, 0.0f),    vec3(0.0f, 1.0f, 0.0f)},
    {vec3(0.0f, 0.1f, 0.1f),    vec2(-1.0f, 0.0f),    vec3(0.0f, 1.0f, 0.0f)},
};

}  // namespace

Mesh::Mesh(const void* data, std::size_t size, bool textured) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &buffer);
    bind();
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, textured ? sizeof(Vertex) : sizeof(vec3), nullptr);
    glEnableVertexAttribArray(0);
    if (textured) {
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, uv)));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
    }
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &buffer);
}

void Mesh::bind() const {
    glBindVertexArray(vao);
}

Mesh Mesh::cube() {
    return Mesh(cubeVertices, sizeof(cubeVertices), true);
}

Mesh Mesh::guides() {
    std::vector<vec3> points = {
        {0, 0, 0}, {0.5f, 0, 0},
        {0, 0, 0}, {0, 0.5f, 0},
        {0, 0, 0}, {0, 0, 0.5f},
    };
    for (int i = 0; i <= gridDivisions; ++i) {
        const float offset = -5.0f + 10.0f * i / gridDivisions;
        points.insert(points.end(), {{offset, 0, -5}, {offset, 0, 5},
                                     {-5, 0, offset}, {5, 0, offset}});
    }
    return Mesh(points.data(), points.size() * sizeof(vec3), false);
}
