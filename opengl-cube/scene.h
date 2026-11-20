#pragma once

#include "camera.h"
#include "mesh.h"
#include "rubiks_cube.h"
#include "shader.h"
#include "texture.h"

#ifdef __EMSCRIPTEN__
#include "landscape.h"
#include "walker.h"
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <array>
#include <vector>

struct LetterPart;

class Scene {
public:
    explicit Scene(GLFWwindow* window);
    ~Scene();
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    void frame();
    void queuePuzzleKey(int key);
#ifdef __EMSCRIPTEN__
    bool toggleWalking();
    void walkLook(float dx, float dy);
#endif

private:
    struct LetterPose {
        glm::vec3 offset{0.0f};
        glm::vec3 rotation{0.0f};
        glm::mat4 shear{1.0f};
        float scale = 1.0f;
    };

    void updateInput(float elapsed);
    void updateLetter(LetterPose& pose, float elapsed, bool fast);
    void onKey(int key);
    void onScroll(double offset);
    void setMouseLook(bool enabled);
    bool down(int key) const;

    glm::mat4 letterModel(std::size_t index) const;
    static glm::mat4 letterPartMatrix(const glm::mat4& model, const LetterPart& part);
#ifdef __EMSCRIPTEN__
    void resetWalker();
    void updateWalking(float elapsed);
#endif

    void drawPlatform(const Shader& shader, bool shadowPass);
    void drawGeometry(const Shader& shader, bool shadowPass);
    void drawPuzzle(const Shader& shader, bool shadowPass);
    void drawBackdrop(const Shader& shader);
    void drawGuides();

    GLFWwindow* window;
    Shader textured;
    Shader shadow;
    Mesh cube = Mesh::cube();
    Mesh guides = Mesh::guides();
    Texture ground;
    Texture box;
    Texture metal;
#ifdef __EMSCRIPTEN__
    Landscape landscape;
    Walker walker;
    CameraState inspectCamera{};
    glm::vec2 inspectOrbit{0.0f};
    bool inspectOrthographic = false;
    bool walking = false;
#endif
    std::vector<Texture> digits;
    std::array<std::vector<Texture>, 6> stickers;
    std::vector<Texture> skybox;

    GLuint shadowFramebuffer = 0;
    GLuint shadowTexture = 0;
    CameraState camera;
    glm::vec2 orbit{0.0f};
    glm::mat4 orientation{1.0f};
    std::array<LetterPose, 5> letters{};
    RubiksCube puzzle;
    int selection = -1;
    bool stepped = false;
    bool texturedModels = true;
    bool orthographic = false;
    bool looking = false;
    std::array<bool, GLFW_KEY_LAST + 1> pressed{};
    double lastFrameTime = 0.0;
    double timerStart = 0.0;
    double cursorX = 0.0;
    double cursorY = 0.0;
};
