#pragma once

#include "shader.h"

class OutdoorSky {
public:
    OutdoorSky();
    ~OutdoorSky();
    OutdoorSky(const OutdoorSky&) = delete;
    OutdoorSky& operator=(const OutdoorSky&) = delete;

    void draw(const glm::mat4& terrainToClip, glm::vec3 sun, float time) const;
    void bindEnvironment() const;
    static constexpr int environmentUnit = 3;

private:
    Shader renderer;
    GLuint cubeMap = 0;
    GLuint vao = 0;
};
