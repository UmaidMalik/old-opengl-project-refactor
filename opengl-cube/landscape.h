#pragma once

#include "shader.h"

#include <vector>

class Landscape {
public:
    struct Tree { glm::vec2 center; float scale; };
    static constexpr float waterLevel = -0.22f;
    static constexpr float walkLimit = 60.0f;
    static const glm::vec3 sunDirection;
    static float heightAt(glm::vec2 point);
    static float pondRadius(glm::vec2 point);

    Landscape();
    ~Landscape();
    Landscape(const Landscape&) = delete;
    Landscape& operator=(const Landscape&) = delete;

    const std::vector<Tree>& trees() const { return forest; }
    void drawSky(const glm::mat4& terrainToClip);
    void draw(const glm::mat4& world, const glm::mat4& viewProjection,
              const glm::mat4& lightSpace, const glm::vec3& camera);
    void drawShadows(const Shader& shadow, const glm::mat4& world) const;

private:
    Shader surfaces;
    Shader sky;
    GLuint vao = 0;
    GLuint buffer = 0;
    GLuint indexBuffer = 0;
    GLsizei terrainVertices = 0;
    GLsizei terrainIndices = 0;
    GLsizei solidCount = 0;
    GLsizei totalCount = 0;
    std::vector<Tree> forest;
};
