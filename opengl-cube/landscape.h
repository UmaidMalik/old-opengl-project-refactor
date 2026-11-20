#pragma once

#include "outdoor_sky.h"
#include "shader.h"

#include <map>
#include <utility>

class Landscape {
public:
    static constexpr float waterLevel = -0.22f;
    static const glm::vec3 sunDirection;
    static float heightAt(glm::vec2 point);
    static float pondRadius(glm::vec2 point);
    static bool hitsTree(glm::vec2 point, float playerRadius);

    Landscape();
    ~Landscape();
    Landscape(const Landscape&) = delete;
    Landscape& operator=(const Landscape&) = delete;

    void stream(glm::vec3 cameraInTerrain);
    void drawSky(const glm::mat4& terrainToClip, float time) const;
    void draw(const glm::mat4& world, const glm::mat4& viewProjection, const glm::mat4& lightSpace,
              const glm::vec3& camera, float time);
    void drawShadows(const Shader& shadow, const glm::mat4& world, const glm::mat4& lightSpace,
                     float time);

private:
    using Key = std::pair<int, int>;

    struct Chunk {
        Chunk() = default;
        ~Chunk();
        Chunk(const Chunk&) = delete;
        Chunk& operator=(const Chunk&) = delete;

        GLuint vao = 0;
        GLuint buffer = 0;
        GLuint grassVao = 0;
        GLuint grassBuffer = 0;
        GLsizei treeVertices = 0;
        GLsizei clumps = 0;
    };

    void buildChunk(Chunk& chunk, Key key);
    void drawChunks(const glm::mat4& terrainToClip) const;
    void drawGrass(Shader& shader, const glm::mat4& world, const glm::mat4& viewProjection,
                   const glm::mat4& lightSpace, float time, float range);

    Shader surfaces;
    OutdoorSky sky;
    Shader foliage;
    Shader foliageShadow;
    GLuint waterVao = 0;
    GLuint waterBuffer = 0;
    GLuint indexBuffer = 0;
    GLuint bladeBuffer = 0;
    GLsizei terrainIndices = 0;
    std::map<Key, Chunk> chunks;
    Key center{};
    glm::vec3 terrainCamera{0.0f};
};
