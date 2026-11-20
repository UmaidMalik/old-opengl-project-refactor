#include "scene.h"
#include "letters.h"

#include <algorithm>
#include <cstdlib>
#include <glm/gtc/matrix_transform.hpp>

namespace {

constexpr int puzzleSelection = 5;
constexpr int turnKeys[2][9] = {
    {GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_T, GLFW_KEY_Y, GLFW_KEY_U, GLFW_KEY_I, GLFW_KEY_O},
    {GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_F, GLFW_KEY_G, GLFW_KEY_H, GLFW_KEY_J, GLFW_KEY_K, GLFW_KEY_L},
};

}  // namespace

bool Scene::down(int key) const {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void Scene::onKey(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) {
        return;
    }
    pressed[key] = true;
    if (looking) {
        return;
    }
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_6) {
        selection = key - GLFW_KEY_1;
    }
    if (selection != puzzleSelection) {
        return;
    }

    for (int row = 0; row < 2; ++row) {
        for (int index = 0; index < 9; ++index) {
            if (key == turnKeys[row][index]) {
                const int axis = index / 3;
                // Keep the original layer order and direction on each axis.
                const int sign = axis == 0 ? 1 : -1;
                puzzle.turn(axis, sign * (index % 3 - 1), row == 0 ? -1 : 1);
            }
        }
    }
}

void Scene::onScroll(double offset) {
    if (looking && offset != 0) {
        camera.fieldOfView = std::clamp(camera.fieldOfView + (offset > 0 ? -5.0f : 5.0f), 10.0f, 125.0f);
    }
}

void Scene::setMouseLook(bool enabled) {
    looking = enabled;
    if (looking) {
        selection = -1;
    }
    glfwSetInputMode(window, GLFW_CURSOR, looking ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    glfwGetCursorPos(window, &cursorX, &cursorY);
}

void Scene::updateInput(float elapsed) {
    if (pressed[GLFW_KEY_ESCAPE]) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
#ifndef __EMSCRIPTEN__
    if (pressed[GLFW_KEY_B]) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (pressed[GLFW_KEY_N]) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (pressed[GLFW_KEY_M]) glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
#endif
    if (pressed[GLFW_KEY_0]) timerStart = lastFrameTime;
    if (pressed[GLFW_KEY_TAB]) stepped = !stepped;
    if (pressed[GLFW_KEY_X]) texturedModels = !texturedModels;
    if (pressed[GLFW_KEY_V]) orthographic = true;
    if (pressed[GLFW_KEY_E] && selection != puzzleSelection) orthographic = false;

    const bool fast = down(GLFW_KEY_LEFT_SHIFT) || down(GLFW_KEY_RIGHT_SHIFT);

    if (looking) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        camera.horizontalAngle -= static_cast<float>(x - cursorX) * 0.25f;
        camera.verticalAngle = std::clamp(camera.verticalAngle - static_cast<float>(y - cursorY) * 0.25f, -85.0f, 85.0f);
        cursorX = x;
        cursorY = y;
        updateCameraDirection(camera);
        const auto side = glm::normalize(glm::cross(camera.direction, camera.up));
        const float speed = elapsed * (fast ? camera.fastSpeed : camera.normalSpeed);
        camera.position += speed * (camera.direction * float(down(GLFW_KEY_W) - down(GLFW_KEY_S))
            + side * float(down(GLFW_KEY_D) - down(GLFW_KEY_A))
            + camera.up * float(down(GLFW_KEY_SPACE) - down(GLFW_KEY_Z)));
    } else if (selection >= 0 && selection < puzzleSelection) {
        updateLetter(letters[selection], elapsed, fast);
    }

    const float step = glm::radians(15.0f);
    orbit.x += step * (pressed[GLFW_KEY_RIGHT] - pressed[GLFW_KEY_LEFT]);
    orbit.y += step * (pressed[GLFW_KEY_UP] - pressed[GLFW_KEY_DOWN]);
    if (pressed[GLFW_KEY_HOME]) {
        camera.position = {0.0f, 4.0f, 3.5f};
        camera.horizontalAngle = 90.0f;
        camera.verticalAngle = 0.0f;
        updateCameraDirection(camera);
        orbit = glm::vec2(0.0f);
    }
    orientation = glm::rotate(glm::mat4(1.0f), -orbit.x, {1, 0, 0});
    orientation = glm::rotate(orientation, -orbit.y, {0, 1, 0});
    pressed.fill(false);
}

void Scene::updateLetter(LetterPose& pose, float elapsed, bool fast) {
    const auto active = [this](int key) { return stepped ? pressed[key] : down(key); };
    const float step = stepped ? 0.1f : elapsed * (fast ? 6.0f : 1.0f);
    const glm::vec3 motion(
        active(GLFW_KEY_D) - active(GLFW_KEY_A),
        active(GLFW_KEY_SPACE) - active(GLFW_KEY_Z),
        active(GLFW_KEY_S) - active(GLFW_KEY_W));
    pose.offset += step * motion;
    pose.scale = std::max(0.05f, pose.scale + step * (active(GLFW_KEY_U) - active(GLFW_KEY_J)));
    pose.shear[1][0] = -motion.x * (fast ? 1.0f : 0.5f);
    pose.shear[1][2] = -motion.z * (fast ? 1.0f : 0.5f);

    pose.rotation += glm::radians(5.0f) * glm::vec3(
        pressed[GLFW_KEY_T] - pressed[GLFW_KEY_Y],
        pressed[GLFW_KEY_G] - pressed[GLFW_KEY_H],
        pressed[GLFW_KEY_B] - pressed[GLFW_KEY_N]);

    if (pressed[GLFW_KEY_LEFT_ALT]) {
        const auto randomCoordinate = [] {
            return static_cast<float>(10.0 * std::rand() / RAND_MAX - 5.0);
        };
        const auto& origin = letterDesigns[selection].position;
        pose.offset = {randomCoordinate() - origin.x, 0.0f, randomCoordinate() - origin.z};
    }
    if (down(GLFW_KEY_R)) {
        if (!fast) pose.offset = glm::vec3(0.0f);
        else if (down(GLFW_KEY_LEFT_CONTROL)) pose.rotation = glm::vec3(0.0f);
        else pose.scale = 1.0f;
    }
}
