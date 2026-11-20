#include "outdoor_sky.h"
#include "asset_paths.h"
#include "texture.h"

#include <algorithm>
#include <stdexcept>

namespace {

constexpr const char* triangle = ASSET_PATH("shaders/fullscreen_vertex.glsl");

}  // namespace

OutdoorSky::OutdoorSky()
    : renderer(triangle, ASSET_PATH("shaders/landscape_sky.glsl")) {
    const Shader projection(triangle, ASSET_PATH("shaders/sky_projection.glsl"));
    glActiveTexture(GL_TEXTURE0);
    const Texture panorama(ASSET_PATH("textures/skybox/temperate-sky.png"));
    glBindTexture(GL_TEXTURE_2D, panorama.getTextureID());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLint supportedSize = 0;
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &supportedSize);
    const GLint size = std::min(1024, supportedSize);
    glGenTextures(1, &cubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    for (int face = 0; face < 6; ++face) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA8, size, size, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glViewport(0, 0, size, size);
    projection.useProgram();
    projection.setInt("panorama", 0);

    for (int face = 0; face < 6; ++face) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, cubeMap, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &framebuffer);
            glDeleteVertexArrays(1, &vao);
            glDeleteTextures(1, &cubeMap);
            throw std::runtime_error("Could not build the outdoor sky cubemap.");
        }
        projection.setInt("face", face);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &framebuffer);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindVertexArray(0);
    renderer.useProgram();
    renderer.setInt("skyMap", environmentUnit);
}

OutdoorSky::~OutdoorSky() {
    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &cubeMap);
}

void OutdoorSky::bindEnvironment() const {
    glActiveTexture(GL_TEXTURE0 + environmentUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
    glActiveTexture(GL_TEXTURE0);
}

void OutdoorSky::draw(const glm::mat4& terrainToClip, glm::vec3 sun, float time) const {
    bindEnvironment();
    renderer.useProgram();
    renderer.setMat4("clipToTerrain", glm::inverse(terrainToClip));
    renderer.setVec3("terrainSun", sun);
    renderer.setFloat("skyTime", time);
    glBindVertexArray(vao);
    glDepthMask(GL_FALSE);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDepthMask(GL_TRUE);
}
