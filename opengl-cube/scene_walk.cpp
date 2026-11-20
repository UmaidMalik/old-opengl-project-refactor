#include "scene.h"
#include "letters.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <emscripten.h>

EM_JS(void, reportPlatformDistance, (int meters), {
    Module['platformDistance'](meters);
});

namespace {

const glm::vec2 spawn(1.3f, 4.15f);

float walkFloor(glm::vec2 p) {
    if (glm::max(std::abs(p.x), std::abs(p.y)) <= 5.0f) return 0.0f;
    return Landscape::heightAt(p);
}

struct Bounds {
    glm::vec3 min;
    glm::vec3 max;
};

Bounds cubeBounds(const glm::mat4& transform) {
    const float limit = std::numeric_limits<float>::max();
    Bounds bounds{glm::vec3(limit), glm::vec3(-limit)};
    for (int corner = 0; corner < 8; ++corner) {
        const auto point = glm::vec3(transform * glm::vec4(
            corner & 1 ? 0.1f : 0.0f, corner & 2 ? 0.1f : 0.0f, corner & 4 ? 0.1f : 0.0f, 1.0f));
        bounds.min = glm::min(bounds.min, point);
        bounds.max = glm::max(bounds.max, point);
    }
    return bounds;
}

bool hitsBox(glm::vec3 feet, const Bounds& box) {
    if (feet.y >= box.max.y - 0.001f || feet.y + Walker::height <= box.min.y) return false;
    const glm::vec2 p(feet.x, feet.z);
    const auto nearest = glm::clamp(p, glm::vec2(box.min.x, box.min.z), glm::vec2(box.max.x, box.max.z));
    const auto offset = p - nearest;
    return glm::dot(offset, offset) < Walker::radius * Walker::radius;
}

}  // namespace

bool Scene::toggleWalking() {
    if (walking) {
        walking = false;
        camera = inspectCamera;
        orbit = inspectOrbit;
        orthographic = inspectOrthographic;
    } else {
        if (looking) setMouseLook(false);
        inspectCamera = camera;
        inspectOrbit = orbit;
        inspectOrthographic = orthographic;
        walking = true;
        orthographic = false;
        orbit = glm::vec2(0.0f);
        orientation = glm::mat4(1.0f);
        selection = -1;
        resetWalker();
    }
    pressed.fill(false);
    return walking;
}

void Scene::resetWalker() {
    walker.reset({spawn.x, walkFloor(spawn), spawn.y});
    camera = initialCamera();
    camera.position = walker.eye();
    camera.horizontalAngle = 118.0f;
    camera.verticalAngle = 15.0f;
    camera.fieldOfView = 73.0f;
    updateCameraDirection(camera);
}

void Scene::walkLook(float dx, float dy) {
    if (!walking) return;
    camera.horizontalAngle = std::remainder(camera.horizontalAngle - dx * 0.16f, 360.0f);
    camera.verticalAngle = std::clamp(camera.verticalAngle - dy * 0.16f, -85.0f, 85.0f);
    updateCameraDirection(camera);
}

void Scene::updateWalking(float elapsed) {
    if (pressed[GLFW_KEY_HOME]) resetWalker();
    walkLook(530.0f * elapsed * (down(GLFW_KEY_RIGHT) - down(GLFW_KEY_LEFT)),
             530.0f * elapsed * (down(GLFW_KEY_DOWN) - down(GLFW_KEY_UP)));

    const glm::vec2 forward = glm::normalize(glm::vec2(camera.direction.x, camera.direction.z));
    const glm::vec2 right(-forward.y, forward.x);
    auto motion = forward * float(down(GLFW_KEY_W) - down(GLFW_KEY_S))
                + right * float(down(GLFW_KEY_D) - down(GLFW_KEY_A));
    const bool fast = down(GLFW_KEY_LEFT_SHIFT) || down(GLFW_KEY_RIGHT_SHIFT);
    if (motion != glm::vec2(0.0f)) motion = glm::normalize(motion) * (fast ? 5.6f : 3.2f);

    std::vector<Bounds> objects;
    objects.reserve(48);
    for (std::size_t i = 0; i < letters.size(); ++i) {
        const auto model = letterModel(i);
        for (auto part = letterDesigns[i].begin; part != letterDesigns[i].end; ++part) {
            objects.push_back(cubeBounds(letterPartMatrix(model, *part)));
        }
    }
    objects.push_back({{-0.47f, 2.53f, 1.53f}, {0.47f, 3.47f, 2.47f}});

    walker.advance(elapsed, motion, pressed[GLFW_KEY_SPACE], [&](glm::vec2 p) {
        float floor = walkFloor(p);
        for (const auto& object : objects) {
            const glm::vec3 candidate(p.x, object.max.y - 0.01f, p.y);
            if (object.max.y <= walker.position().y + 0.025f && hitsBox(candidate, object)) {
                floor = std::max(floor, object.max.y);
            }
        }
        return floor;
    }, [&](glm::vec3 feet) {
        const glm::vec2 p(feet.x, feet.z);
        if (Landscape::pondRadius(p) < 1.18f && walkFloor(p) < Landscape::waterLevel - 0.36f) return true;
        if (Landscape::hitsTree(p, Walker::radius)) return true;
        for (const auto& object : objects) {
            if (hitsBox(feet, object)) return true;
        }
        return false;
    });
    camera.position = walker.eye();
    const auto feet = walker.position();
    const int meters = static_cast<int>(glm::length(glm::vec2(feet.x, feet.z)));
    if (meters != lastPlatformDistance) {
        lastPlatformDistance = meters;
        reportPlatformDistance(meters);
    }
}
