#include "landscape.h"
#include "asset_paths.h"

#include <algorithm>
#include <stdexcept>

namespace {

constexpr const char* fullscreenShader = ASSET_PATH("shaders/fullscreen_vertex.glsl");

void configureTexture(GLuint texture, GLint filter) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

}  // namespace

const glm::vec3 Landscape::sunDirection = glm::normalize(glm::vec3(-0.58f, 0.23f, -0.78f));

Landscape::Landscape()
    : trace(fullscreenShader, ASSET_PATH("shaders/landscape_fragment.glsl")),
      composite(fullscreenShader, ASSET_PATH("shaders/landscape_composite.glsl")) {
    glGenFramebuffers(1, &framebuffer);
    glGenTextures(1, &color);
    glGenTextures(1, &depth);
    glGenVertexArrays(1, &vao);

    trace.useProgram();
    trace.setVec3("sunDirection", sunDirection);
    composite.useProgram();
    composite.setInt("landscapeColor", 0);
    composite.setInt("landscapeDepth", 2);
}

Landscape::~Landscape() {
    glDeleteVertexArrays(1, &vao);
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &color);
    glDeleteTextures(1, &depth);
}

void Landscape::resize(int width, int height) {
    traceWidth = width;
    traceHeight = height;
    current = false;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    configureTexture(color, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    configureTexture(depth, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
    const auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Could not create the landscape framebuffer.");
    }
}

void Landscape::draw(const glm::mat4& terrainToClip, int width, int height) {
    constexpr int maxTraceWidth = 640;
    const int rayWidth = std::clamp(width / 2, 1, maxTraceWidth);
    const int rayHeight = std::max(1, height * rayWidth / width);

    glActiveTexture(GL_TEXTURE0);
    if (rayWidth != traceWidth || rayHeight != traceHeight) {
        resize(rayWidth, rayHeight);
    }

    glBindVertexArray(vao);
    glDepthFunc(GL_ALWAYS);
    // Camera and projection changes invalidate the landscape. Foreground animation can reuse it.
    if (!current || terrainToClip != lastTerrainToClip) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glViewport(0, 0, traceWidth, traceHeight);
        trace.useProgram();
        trace.setMat4("clipToTerrain", glm::inverse(terrainToClip));
        trace.setMat4("terrainToClip", terrainToClip);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        lastTerrainToClip = terrainToClip;
        current = true;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    composite.useProgram();
    glBindTexture(GL_TEXTURE_2D, color);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depth);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDepthFunc(GL_LESS);
}
