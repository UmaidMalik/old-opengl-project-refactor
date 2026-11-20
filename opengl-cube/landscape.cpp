#include "landscape.h"
#include "asset_paths.h"

#include <cmath>
#include <cstddef>
#include <glm/gtc/constants.hpp>

namespace {

constexpr float spacing = 0.75f;
constexpr int halfGrid = 104;
constexpr float cellSize = 5.0f;
const glm::vec2 pondCenter(-1.0f, -16.0f);
const glm::vec2 pondRadii(5.0f, 4.0f);

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    float water = 0.0f;
};

float hash(glm::vec2 p) {
    auto q = glm::fract(glm::vec3(p.x, p.y, p.x) * glm::vec3(0.1031f, 0.1030f, 0.0973f));
    q += glm::dot(q, glm::vec3(q.y, q.z, q.x) + 33.33f);
    return glm::fract((q.x + q.y) * q.z);
}

float noise(glm::vec2 p) {
    const auto cell = glm::floor(p);
    auto f = glm::fract(p);
    f = f * f * (3.0f - 2.0f * f);
    return glm::mix(glm::mix(hash(cell), hash(cell + glm::vec2(1, 0)), f.x),
                    glm::mix(hash(cell + glm::vec2(0, 1)), hash(cell + 1.0f), f.x), f.y);
}

float terrain(glm::vec2 p) {
    const float clearing = glm::smoothstep(5.25f, 12.5f, glm::max(std::abs(p.x), std::abs(p.y)));
    float hills = 5.0f * noise(p * 0.083f + 6.2f) - 1.55f + noise(p * 0.24f + 14.7f);
    hills += glm::smoothstep(22.0f, 88.0f, glm::length(p)) * (3.0f + 9.0f * noise(p * 0.049f - 3.0f));
    const float height = -0.35f + clearing * hills;
    return glm::mix(-1.6f, height, glm::smoothstep(0.69f, 1.27f, Landscape::pondRadius(p)));
}

Vertex grass(glm::vec2 p) {
    const float height = terrain(p);
    constexpr float e = 0.25f;
    const auto normal = glm::normalize(glm::vec3(
        terrain(p - glm::vec2(e, 0)) - terrain(p + glm::vec2(e, 0)), 2 * e,
        terrain(p - glm::vec2(0, e)) - terrain(p + glm::vec2(0, e))));
    const float variation = noise(p * 0.43f) * 0.76f + noise(p * 1.1f) * 0.24f;
    auto color = glm::mix(glm::vec3(0.14f, 0.28f, 0.09f), glm::vec3(0.45f, 0.53f, 0.21f), variation);
    const float sand = (1.0f - glm::smoothstep(0.03f, 0.36f, std::abs(height - Landscape::waterLevel)))
                       * (1.0f - glm::smoothstep(1.2f, 1.3f, Landscape::pondRadius(p)));
    color = glm::mix(color, glm::vec3(0.46f, 0.40f, 0.25f), sand);
    // Bake a little valley and slope shading into the vertex colors.
    color *= glm::mix(0.76f, 1.0f, normal.y) * glm::mix(0.86f, 1.0f, noise(p * 0.115f));
    return {{p.x, height, p.y}, normal, color};
}

void branchMesh(std::vector<Vertex>& vertices, glm::vec3 base, float height, float radius,
                float topRadius, glm::vec3 color, int sides) {
    const float slope = (radius - topRadius) / height;
    const glm::vec3 tip = base + glm::vec3(0, height, 0);
    for (int side = 0; side < sides; ++side) {
        const float angle = glm::two_pi<float>() * side / sides;
        const float next = glm::two_pi<float>() * (side + 1) / sides;
        const glm::vec3 radial(std::cos(angle), 0, std::sin(angle));
        const glm::vec3 nextRadial(std::cos(next), 0, std::sin(next));
        const auto n = glm::normalize(radial + glm::vec3(0, slope, 0));
        const auto nn = glm::normalize(nextRadial + glm::vec3(0, slope, 0));
        const Vertex a{base + radius * radial, n, color * 0.82f};
        const Vertex b{base + radius * nextRadial, nn, color * 0.82f};
        const Vertex c{tip + topRadius * radial, n, color};
        const Vertex d{tip + topRadius * nextRadial, nn, color};
        vertices.insert(vertices.end(), {a, b, c});
        if (topRadius > 0) vertices.insert(vertices.end(), {b, d, c});
        const glm::vec3 down(0, -1, 0);
        vertices.insert(vertices.end(), {{base, down, color * 0.61f},
                          {b.position, down, b.color}, {a.position, down, a.color}});
    }
}

}  // namespace

const glm::vec3 Landscape::sunDirection = glm::normalize(glm::vec3(-0.57f, 0.44f, -0.72f));

float Landscape::pondRadius(glm::vec2 point) {
    return glm::length((point - pondCenter) / pondRadii);
}

float Landscape::heightAt(glm::vec2 p) {
    const auto grid = p / spacing;
    const auto base = glm::floor(grid) * spacing;
    const auto f = glm::fract(grid);
    const float b = terrain(base + glm::vec2(spacing, 0));
    const float c = terrain(base + glm::vec2(0, spacing));
    if (f.x + f.y <= 1.0f) {
        return terrain(base) * (1.0f - f.x - f.y) + b * f.x + c * f.y;
    }
    return terrain(base + spacing) * (f.x + f.y - 1.0f) + b * (1.0f - f.y) + c * (1.0f - f.x);
}

Landscape::Landscape()
    : surfaces(ASSET_PATH("shaders/landscape_vertex.glsl"), ASSET_PATH("shaders/landscape_fragment.glsl")),
      sky(ASSET_PATH("shaders/fullscreen_vertex.glsl"), ASSET_PATH("shaders/landscape_sky.glsl")) {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    constexpr int gridWidth = halfGrid * 2 + 1;
    vertices.reserve(gridWidth * gridWidth);
    indices.reserve(6 * (gridWidth - 1) * (gridWidth - 1));
    for (int z = -halfGrid; z <= halfGrid; ++z) {
        for (int x = -halfGrid; x <= halfGrid; ++x) vertices.push_back(grass({x * spacing, z * spacing}));
    }
    for (int z = 0; z < gridWidth - 1; ++z) {
        for (int x = 0; x < gridWidth - 1; ++x) {
            const auto i = static_cast<GLuint>(z * gridWidth + x);
            indices.insert(indices.end(), {i, i + 1, i + gridWidth,
                                          i + 1, i + gridWidth + 1, i + gridWidth});
        }
    }
    terrainVertices = static_cast<GLsizei>(vertices.size());
    terrainIndices = static_cast<GLsizei>(indices.size());

    for (int z = -15; z < 15; ++z) {
        for (int x = -15; x < 15; ++x) {
            const glm::vec2 cell(x, z);
            if (hash(cell + 27.3f) <= 0.40f) continue;
            const glm::vec2 jitter(hash(cell + 17.0f) - 0.5f, hash(cell - 63.0f) - 0.5f);
            const auto center = (cell + 0.5f) * cellSize + jitter * 1.8f;
            if (glm::max(std::abs(center.x), std::abs(center.y)) < 7.6f || pondRadius(center) < 1.21f) continue;
            const float scale = glm::mix(0.68f, 1.27f, hash(cell - 3.0f));
            forest.push_back({center, scale});
            const glm::vec3 base(center.x, heightAt(center) - 0.06f, center.y);
            const float h = 4.1f * scale;
            const auto needles = glm::mix(glm::vec3(0.09f, 0.22f, 0.12f),
                                          glm::vec3(0.22f, 0.36f, 0.17f), hash(cell + 7.1f));
            branchMesh(vertices, base, h * 0.65f, 0.12f * scale, 0.09f * scale,
                       {0.29f, 0.19f, 0.105f}, 6);
            branchMesh(vertices, base + glm::vec3(0, h * 0.19f, 0), h * 0.55f, 1.25f * scale, 0, needles, 8);
            branchMesh(vertices, base + glm::vec3(0, h * 0.36f, 0), h * 0.48f, 1.01f * scale, 0, needles, 8);
            branchMesh(vertices, base + glm::vec3(0, h * 0.53f, 0), h * 0.47f, 0.73f * scale, 0, needles, 8);
        }
    }
    solidCount = static_cast<GLsizei>(vertices.size());

    // Terrain depth naturally clips the pond surface at the shoreline.
    for (glm::vec2 p : {glm::vec2(-1, -1), {1, -1}, {-1, 1}, {1, -1}, {1, 1}, {-1, 1}}) {
        const auto corner = pondCenter + p * pondRadii * 1.18f;
        vertices.push_back({{corner.x, waterLevel, corner.y}, {0, 1, 0}, {0.10f, 0.26f, 0.28f}, 1.0f});
    }
    totalCount = static_cast<GLsizei>(vertices.size());

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    const std::size_t offsets[] = {offsetof(Vertex, position), offsetof(Vertex, normal),
                                   offsetof(Vertex, color), offsetof(Vertex, water)};
    for (GLuint i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, i == 3 ? 1 : 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsets[i]));
    }
    surfaces.useProgram();
    surfaces.setInt("shadowMap", 1);
    sky.useProgram();
    sky.setVec3("sunDirection", sunDirection);
}

Landscape::~Landscape() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &indexBuffer);
    glDeleteBuffers(1, &buffer);
}

void Landscape::drawSky(const glm::mat4& terrainToClip) {
    sky.useProgram();
    sky.setMat4("clipToTerrain", glm::inverse(terrainToClip));
    glBindVertexArray(vao);
    glDepthMask(GL_FALSE);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDepthMask(GL_TRUE);
}

void Landscape::draw(const glm::mat4& world, const glm::mat4& viewProjection,
                     const glm::mat4& lightSpace, const glm::vec3& camera) {
    surfaces.useProgram();
    surfaces.setMat4("worldMatrix", world);
    surfaces.setMat4("viewProjection", viewProjection);
    surfaces.setMat4("lightSpace", lightSpace);
    surfaces.setVec3("viewPos", camera);
    surfaces.setVec3("sunDirection", glm::vec3(world * glm::vec4(sunDirection, 0.0f)));
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, terrainIndices, GL_UNSIGNED_INT, nullptr);
    glDrawArrays(GL_TRIANGLES, terrainVertices, totalCount - terrainVertices);
}

void Landscape::drawShadows(const Shader& shadow, const glm::mat4& world) const {
    shadow.setMat4("worldMatrix", world);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, terrainIndices, GL_UNSIGNED_INT, nullptr);
    glDrawArrays(GL_TRIANGLES, terrainVertices, solidCount - terrainVertices);
}
