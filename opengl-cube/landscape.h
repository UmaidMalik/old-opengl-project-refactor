#pragma once

#include "shader.h"

class Landscape {
public:
    Landscape();
    ~Landscape();
    Landscape(const Landscape&) = delete;
    Landscape& operator=(const Landscape&) = delete;

    void draw(const glm::mat4& terrainToClip, int width, int height);
    static const glm::vec3 sunDirection;

private:
    void resize(int width, int height);

    Shader trace;
    Shader composite;
    GLuint framebuffer = 0;
    GLuint color = 0;
    GLuint depth = 0;
    GLuint vao = 0;
    int traceWidth = 0;
    int traceHeight = 0;
    glm::mat4 lastTerrainToClip{0.0f};
    bool current = false;
};
