#include "landscape.h"
#include "asset_paths.h"

#include <cmath>
#include <cstddef>
#include <algorithm>
#include <vector>
#include <glm/gtc/constants.hpp>

namespace {

constexpr float spacing = 0.75f;
constexpr int quads = 32;
constexpr int gridWidth = quads + 1;
constexpr int gridVertices = gridWidth * gridWidth;
constexpr float chunkSize = spacing * quads;
constexpr int viewRadius = 3;
constexpr float bladeRange = 32.0f;
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
    hills += 4.0f * (noise(p * 0.017f + 7.1f) - 0.5f)
           * glm::smoothstep(25.0f, 90.0f, glm::length(p));
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
    const float broadShade = terrain(p + glm::vec2(1.7f, -1.4f)) - height;
    const float ambientOcclusion = 1.0f - 0.22f * glm::smoothstep(0.12f, 1.35f, broadShade);
    color *= ambientOcclusion * glm::mix(0.79f, 1.0f, normal.y);
    return {{p.x, height, p.y}, normal, color};
}

void branchMesh(std::vector<Vertex>& vertices, glm::vec3 base, float height, float radius,
                float topRadius, glm::vec3 color, int sides) {
    const float slope = (radius - topRadius) / height;
    const glm::vec3 tip = base + glm::vec3(0, height, 0);
    for (int side = 0; side < sides; ++side) {
        const float twist = topRadius == 0.0f ? base.y * 4.3f : 0.0f;
        const float angle = glm::two_pi<float>() * side / sides + twist;
        const float next = glm::two_pi<float>() * (side + 1) / sides + twist;
        const glm::vec3 radial(std::cos(angle), 0, std::sin(angle));
        const glm::vec3 nextRadial(std::cos(next), 0, std::sin(next));
        const auto n = glm::normalize(radial + glm::vec3(0, slope, 0));
        const auto nn = glm::normalize(nextRadial + glm::vec3(0, slope, 0));
        const auto skirt = [&](glm::vec3 direction) {
            if (topRadius > 0.0f) return radius * direction;
            const float branch = hash(glm::vec2(base.x, base.z) + glm::vec2(direction.x, direction.z) * 7.0f);
            return radius * (direction * glm::mix(0.78f, 1.13f, branch) + glm::vec3(0, branch * 0.14f, 0));
        };
        const Vertex a{base + skirt(radial), n, color * (0.68f + 0.24f * std::abs(radial.x))};
        const Vertex b{base + skirt(nextRadial), nn, color * (0.68f + 0.24f * std::abs(nextRadial.x))};
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

const glm::vec3 Landscape::sunDirection = glm::normalize(glm::vec3(-0.61f, 0.27f, -0.65f));

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


namespace {

struct Tree {
    glm::vec2 center;
    float scale;
};

Tree treeAt(glm::vec2 cell) {
    const glm::vec2 jitter(hash(cell + 17.0f) - 0.5f, hash(cell - 63.0f) - 0.5f);
    const auto center = (cell + 0.5f) * cellSize + jitter * 1.8f;
    if (hash(cell + 27.3f) <= 0.40f ||
        glm::max(std::abs(center.x), std::abs(center.y)) < 7.6f ||
        Landscape::pondRadius(center) < 1.21f) return {center, 0.0f};
    const float density = noise(center * 0.046f + 21.0f);
    if (glm::length(center) > 28.0f && density < 0.25f) return {center, 0.0f};
    return {center, glm::mix(0.68f, 1.44f, hash(cell - 3.0f))};
}

void uploadSurface(GLuint& vao, GLuint& buffer, const std::vector<Vertex>& vertices) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    const std::size_t offsets[] = {offsetof(Vertex, position), offsetof(Vertex, normal),
                                  offsetof(Vertex, color), offsetof(Vertex, water)};
    for (GLuint i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, i == 3 ? 1 : 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void*>(offsets[i]));
    }
}

bool inFrustum(const glm::mat4& clip, glm::vec2 origin) {
    const glm::mat4 rows = glm::transpose(clip);
    const glm::vec3 center(origin.x + chunkSize * 0.5f, 12.0f, origin.y + chunkSize * 0.5f);
    const glm::vec3 halfSize(chunkSize * 0.5f + 3.0f, 16.0f, chunkSize * 0.5f + 3.0f);
    for (int axis = 0; axis < 3; ++axis) {
        for (const auto& plane : {rows[3] + rows[axis], rows[3] - rows[axis]}) {
            const glm::vec3 n(plane);
            if (glm::dot(n, center) + plane.w + glm::dot(glm::abs(n), halfSize) < 0.0f) return false;
        }
    }
    return true;
}

}  // namespace

bool Landscape::hitsTree(glm::vec2 point, float playerRadius) {
    const auto cell = glm::floor(point / cellSize);
    for (int z = -1; z <= 1; ++z) {
        for (int x = -1; x <= 1; ++x) {
            const auto tree = treeAt(cell + glm::vec2(x, z));
            if (tree.scale == 0.0f) continue;
            const auto offset = point - tree.center;
            const float radius = playerRadius + tree.scale * 0.14f;
            if (glm::dot(offset, offset) < radius * radius) return true;
        }
    }
    return false;
}

Landscape::Landscape()
    : surfaces(ASSET_PATH("shaders/landscape_vertex.glsl"), ASSET_PATH("shaders/landscape_fragment.glsl")),
      foliage(ASSET_PATH("shaders/grass_vertex.glsl"), ASSET_PATH("shaders/grass_fragment.glsl")),
      foliageShadow(ASSET_PATH("shaders/grass_vertex.glsl"), ASSET_PATH("shaders/shadow_fragment.glsl")) {
    std::vector<GLuint> indices;
    indices.reserve(6 * quads * quads);
    for (int z = 0; z < quads; ++z) {
        for (int x = 0; x < quads; ++x) {
            const auto i = static_cast<GLuint>(z * gridWidth + x);
            indices.insert(indices.end(), {i, i + 1, i + gridWidth,
                                          i + 1, i + gridWidth + 1, i + gridWidth});
        }
    }

    std::vector<Vertex> water;
    for (glm::vec2 p : {glm::vec2(-1, -1), {1, -1}, {-1, 1}, {1, -1}, {1, 1}, {-1, 1}}) {
        const auto corner = pondCenter + p * pondRadii * 1.18f;
        water.push_back({{corner.x, waterLevel, corner.y}, {0, 1, 0}, {0.10f, 0.26f, 0.28f}, 1.0f});
    }
    uploadSurface(waterVao, waterBuffer, water);
    terrainIndices = static_cast<GLsizei>(indices.size());
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    const glm::vec2 blade[] = {
        {-1, 0}, {1, 0}, {-0.66f, 0.48f}, {1, 0}, {0.66f, 0.48f}, {-0.66f, 0.48f},
        {-0.66f, 0.48f}, {0.66f, 0.48f}, {0, 1}
    };
    std::vector<glm::vec2> tuft;
    for (int stalk = 0; stalk < 3; ++stalk) {
        tuft.insert(tuft.end(), blade, blade + 9);
    }
    glGenBuffers(1, &bladeBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, bladeBuffer);
    glBufferData(GL_ARRAY_BUFFER, tuft.size() * sizeof(glm::vec2), tuft.data(), GL_STATIC_DRAW);
    surfaces.useProgram();
    surfaces.setInt("shadowMap", 1);
    foliage.useProgram();
    foliage.setInt("shadowMap", 1);
    surfaces.useProgram();
    surfaces.setInt("skyMap", OutdoorSky::environmentUnit);
}

Landscape::Chunk::~Chunk() {
    glDeleteVertexArrays(1, &vao);
    glDeleteVertexArrays(1, &grassVao);
    glDeleteBuffers(1, &buffer);
    glDeleteBuffers(1, &grassBuffer);
}

Landscape::~Landscape() {
    chunks.clear();
    glDeleteVertexArrays(1, &waterVao);
    glDeleteBuffers(1, &waterBuffer);
    glDeleteBuffers(1, &indexBuffer);
    glDeleteBuffers(1, &bladeBuffer);
}

void Landscape::buildChunk(Chunk& chunk, Key key) {
    const glm::vec2 origin(key.first * chunkSize, key.second * chunkSize);
    std::vector<Vertex> vertices;
    vertices.reserve(gridVertices + 3600);
    for (int z = 0; z < gridWidth; ++z) {
        for (int x = 0; x < gridWidth; ++x) vertices.push_back(grass(origin + glm::vec2(x, z) * spacing));
    }
    const auto firstCell = glm::ivec2(glm::floor(origin / cellSize));
    const auto lastCell = glm::ivec2(glm::floor((origin + chunkSize) / cellSize));
    for (int z = firstCell.y; z <= lastCell.y; ++z) {
        for (int x = firstCell.x; x <= lastCell.x; ++x) {
            const glm::vec2 cell(x, z);
            const auto tree = treeAt(cell);
            if (tree.scale == 0.0f || glm::any(glm::lessThan(tree.center, origin)) ||
                glm::any(glm::greaterThanEqual(tree.center, origin + chunkSize))) continue;
            const glm::vec3 base(tree.center.x, heightAt(tree.center) - 0.06f, tree.center.y);
            const float h = 4.1f * tree.scale;
            const auto needles = glm::mix(glm::vec3(0.095f, 0.17f, 0.117f),
                                          glm::vec3(0.20f, 0.285f, 0.158f), hash(cell + 7.1f));
            branchMesh(vertices, base, h * 0.65f, 0.12f * tree.scale, 0.09f * tree.scale, {0.29f, 0.19f, 0.105f}, 7);
            branchMesh(vertices, base + glm::vec3(0, h * 0.20f, 0), h * 0.54f, 1.17f * tree.scale, 0, needles, 11);
            branchMesh(vertices, base + glm::vec3(0.09f, h * 0.34f, 0), h * 0.48f, 1.04f * tree.scale, 0, needles, 11);
            branchMesh(vertices, base + glm::vec3(0, h * 0.49f, -0.07f), h * 0.40f, 0.80f * tree.scale, 0, needles, 11);
            branchMesh(vertices, base + glm::vec3(0.1f, h * 0.65f, 0), h * 0.35f, 0.57f * tree.scale, 0, needles, 10);
            const float heading = hash(cell + 63.0f) * glm::two_pi<float>();
            for (int bough = 0; bough < 5; ++bough) {
                const float detail = hash(cell + static_cast<float>(bough) * 7.8f + 14.4f);
                const float angle = heading + bough * 2.39996f;
                const float reach = tree.scale * glm::mix(0.65f, 0.98f, detail);
                const glm::vec3 offset(std::cos(angle) * reach,
                                       h * glm::mix(0.18f, 0.39f, detail),
                                       std::sin(angle) * reach);
                branchMesh(vertices, base + offset, h * glm::mix(0.12f, 0.20f, detail),
                           tree.scale * glm::mix(0.28f, 0.46f, detail), 0,
                           needles * glm::mix(0.84f, 1.07f, detail), 6);
            }
        }
    }
    chunk.treeVertices = static_cast<GLsizei>(vertices.size()) - gridVertices;
    uploadSurface(chunk.vao, chunk.buffer, vertices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

    constexpr int scatter = 48;
    std::vector<glm::vec4> clumps;
    clumps.reserve(scatter * scatter);
    for (int z = 0; z < scatter; ++z) {
        for (int x = 0; x < scatter; ++x) {
            const auto cell = glm::vec2(key.first * scatter + x, key.second * scatter + z);
            const float seed = hash(cell + 41.3f);
            const glm::vec2 jitter(hash(cell + 4.0f), hash(cell + 19.0f));
            const auto p = (cell + jitter * 0.88f + 0.06f) * (chunkSize / scatter);
            const float patch = noise(p * 0.31f + 8.0f);
            if (seed > glm::mix(0.34f, 0.98f, patch) ||
                glm::max(std::abs(p.x), std::abs(p.y)) < 5.24f) continue;
            const float altitude = heightAt(p);
            if (pondRadius(p) < 1.24f && altitude < waterLevel + 0.12f) continue;
            clumps.emplace_back(p.x, altitude - 0.012f, p.y, seed);
        }
    }
    chunk.clumps = static_cast<GLsizei>(clumps.size());
    glGenVertexArrays(1, &chunk.grassVao);
    glBindVertexArray(chunk.grassVao);
    glGenBuffers(1, &chunk.grassBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, chunk.grassBuffer);
    glBufferData(GL_ARRAY_BUFFER, clumps.size() * sizeof(glm::vec4), clumps.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);
    glVertexAttribDivisor(0, 1);
    glBindBuffer(GL_ARRAY_BUFFER, bladeBuffer);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
}

void Landscape::stream(glm::vec3 cameraInTerrain) {
    terrainCamera = cameraInTerrain;
    const Key next(static_cast<int>(std::floor(cameraInTerrain.x / chunkSize)),
                   static_cast<int>(std::floor(cameraInTerrain.z / chunkSize)));
    if (!chunks.empty() && center == next) return;
    center = next;
    for (auto it = chunks.begin(); it != chunks.end(); ) {
        const auto [x, z] = it->first;
        if (std::abs(x - center.first) > viewRadius || std::abs(z - center.second) > viewRadius) {
            it = chunks.erase(it);
        } else {
            ++it;
        }
    }

    for (int z = center.second - viewRadius; z <= center.second + viewRadius; ++z) {
        for (int x = center.first - viewRadius; x <= center.first + viewRadius; ++x) {
            auto [it, created] = chunks.try_emplace(Key{x, z});
            if (created) buildChunk(it->second, it->first);
        }
    }
}

void Landscape::drawChunks(const glm::mat4& terrainToClip) const {
    for (const auto& [key, chunk] : chunks) {
        const glm::vec2 origin(key.first * chunkSize, key.second * chunkSize);
        if (!inFrustum(terrainToClip, origin)) continue;
        glBindVertexArray(chunk.vao);
        glDrawElements(GL_TRIANGLES, terrainIndices, GL_UNSIGNED_INT, nullptr);
        glDrawArrays(GL_TRIANGLES, gridVertices, chunk.treeVertices);
    }
}

void Landscape::drawGrass(Shader& shader, const glm::mat4& world, const glm::mat4& viewProjection,
                          const glm::mat4& lightSpace, float time, float range) {
    const auto clip = viewProjection * world;
    shader.useProgram();
    shader.setMat4("worldMatrix", world);
    shader.setMat4("viewProjection", viewProjection);
    shader.setMat4("lightSpace", lightSpace);
    shader.setVec3("terrainCamera", terrainCamera);
    shader.setVec3("sunDirection", glm::vec3(world * glm::vec4(sunDirection, 0.0f)));
    shader.setFloat("time", time);
    shader.setFloat("skyTime", time);
    shader.setVec3("terrainSun", sunDirection);
    shader.setFloat("drawRange", range);
    const auto culling = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    const glm::vec2 camera(terrainCamera.x, terrainCamera.z);
    for (const auto& [key, chunk] : chunks) {
        const glm::vec2 origin(key.first * chunkSize, key.second * chunkSize);
        const auto nearest = glm::clamp(camera, origin, origin + chunkSize);
        const auto offset = camera - nearest;
        if (glm::dot(offset, offset) > range * range || !inFrustum(clip, origin)) continue;
        glBindVertexArray(chunk.grassVao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 27, chunk.clumps);
    }
    if (culling) glEnable(GL_CULL_FACE);
}

void Landscape::drawSky(const glm::mat4& terrainToClip, float time) const {
    sky.draw(terrainToClip, sunDirection, time);
}

void Landscape::draw(const glm::mat4& world, const glm::mat4& viewProjection,
                     const glm::mat4& lightSpace, const glm::vec3& camera, float time) {
    surfaces.useProgram();
    surfaces.setMat4("worldMatrix", world);
    surfaces.setMat4("viewProjection", viewProjection);
    surfaces.setMat4("lightSpace", lightSpace);
    surfaces.setVec3("viewPos", camera);
    surfaces.setVec3("terrainCamera", terrainCamera);
    surfaces.setVec3("terrainSun", sunDirection);
    surfaces.setFloat("time", time);
    surfaces.setFloat("skyTime", time);
    surfaces.setVec3("sunDirection", glm::vec3(world * glm::vec4(sunDirection, 0.0f)));
    drawChunks(viewProjection * world);
    glBindVertexArray(waterVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    drawGrass(foliage, world, viewProjection, lightSpace, time, bladeRange);
}

void Landscape::drawShadows(const Shader& shadow, const glm::mat4& world, const glm::mat4& lightSpace,
                            float time) {
    shadow.setMat4("worldMatrix", world);
    drawChunks(lightSpace * world);
    drawGrass(foliageShadow, world, lightSpace, lightSpace, time, 15.0f);
}
