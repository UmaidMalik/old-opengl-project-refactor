#include "scene.h"
#include "asset_paths.h"
#include "letters.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <glm/gtc/matrix_transform.hpp>

namespace {

constexpr int shadowSize = 1024;
#ifdef __EMSCRIPTEN__
constexpr bool studioBackdrop = true;
#else
constexpr bool studioBackdrop = false;
#endif
const glm::mat4 identity(1.0f);
const glm::vec3 lightPosition(-3.0f, 3.0f, 4.0f);

void loadTiles(std::vector<Texture>& tiles, const char* prefix, const char* extension, int first, int last) {
    tiles.reserve(last - first + 1);
    for (int i = first; i <= last; ++i) {
        const auto path = prefix + std::to_string(i) + extension;
        tiles.emplace_back(path.c_str());
    }
}

}  // namespace

Scene::Scene(GLFWwindow* window)
    : window(window),
      textured(ASSET_PATH("shaders/textured_vertex.glsl"), ASSET_PATH("shaders/textured_fragment.glsl")),
      shadow(ASSET_PATH("shaders/shadow_vertex.glsl"), ASSET_PATH("shaders/shadow_fragment.glsl")),
      ground(ASSET_PATH("textures/tiles.bmp")),
      box(ASSET_PATH("textures/box.jpg")),
      metal(ASSET_PATH("textures/metal.jpg")),
      camera(initialCamera()) {
    loadTiles(digits, ASSET_PATH("textures/face_5/number_"), ".png", 0, 9);
    loadTiles(stickers[1], ASSET_PATH("textures/face_6/science_"), ".jpg", 1, 9);
    loadTiles(stickers[2], ASSET_PATH("textures/face_2/color_"), ".png", 1, 9);
    loadTiles(stickers[3], ASSET_PATH("textures/face_1/hollow_knight_art_"), ".png", 1, 9);
    loadTiles(stickers[4], ASSET_PATH("textures/face_4/shape_B_"), ".png", 1, 9);
    loadTiles(stickers[5], ASSET_PATH("textures/face_3/shape_A_"), ".png", 1, 9);
    if constexpr (!studioBackdrop) {
        skybox.reserve(6);
        for (const char* face : {"front", "right", "back", "left", "bottom", "top"}) {
            const auto path = std::string(ASSET_PATH("textures/skybox/skybox_")) + face + ".png";
            skybox.emplace_back(path.c_str());
        }
    }

    glGenTextures(1, &shadowTexture);
    glBindTexture(GL_TEXTURE_2D, shadowTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowSize, shadowSize, 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &shadowFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture, 0);
    const GLenum none = GL_NONE;
    glDrawBuffers(1, &none);
    glReadBuffer(GL_NONE);
    const auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        glDeleteFramebuffers(1, &shadowFramebuffer);
        glDeleteTextures(1, &shadowTexture);
        throw std::runtime_error("Could not create the shadow framebuffer.");
    }

    textured.useProgram();
    textured.setInt("texture_1", 0);
    textured.setInt("shadowMap", 1);
    textured.setVec3("lightColor", glm::vec3(1.0f));
    textured.setFloat("light_cutoff_inner", std::cos(glm::radians(62.5f)));
    textured.setFloat("light_cutoff_outer", std::cos(glm::radians(67.5f)));

    if constexpr (studioBackdrop) glClearColor(0.031f, 0.044f, 0.068f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CW);
#ifndef __EMSCRIPTEN__
    glPointSize(10.0f);
#endif
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, [](GLFWwindow* source, int key, int, int action, int) {
        if (action == GLFW_PRESS) {
            static_cast<Scene*>(glfwGetWindowUserPointer(source))->onKey(key);
        }
    });
    glfwSetMouseButtonCallback(window, [](GLFWwindow* source, int button, int action, int) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            static_cast<Scene*>(glfwGetWindowUserPointer(source))->setMouseLook(action == GLFW_PRESS);
        }
    });
    glfwSetScrollCallback(window, [](GLFWwindow* source, double, double offset) {
        static_cast<Scene*>(glfwGetWindowUserPointer(source))->onScroll(offset);
    });
    timerStart = lastFrameTime = glfwGetTime();
}

Scene::~Scene() {
    glfwSetKeyCallback(window, nullptr);
    glfwSetScrollCallback(window, nullptr);
    glfwSetMouseButtonCallback(window, nullptr);
    glfwSetWindowUserPointer(window, nullptr);
    glDeleteFramebuffers(1, &shadowFramebuffer);
    glDeleteTextures(1, &shadowTexture);
}

void Scene::frame() {
    glfwPollEvents();
    const double now = glfwGetTime();
    const auto elapsed = static_cast<float>(std::min(now - lastFrameTime, 0.1));
    lastFrameTime = now;
    updateInput(elapsed);
    puzzle.advance(elapsed);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (width <= 0 || height <= 0) {
        return;
    }

    if (down(GLFW_KEY_C)) glDisable(GL_CULL_FACE);
    else glEnable(GL_CULL_FACE);
    cube.bind();
    const auto lightView = glm::lookAt(lightPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const auto lightSpace = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 20.0f) * lightView;

    shadow.useProgram();
    shadow.setMat4("light_view_proj_matrix", lightSpace);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glViewport(0, 0, shadowSize, shadowSize);
    glClear(GL_DEPTH_BUFFER_BIT);
    drawGeometry(shadow, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const float aspect = static_cast<float>(width) / height;
    const auto projection = createProjectionMatrix(camera, aspect, orthographic);

    textured.useProgram();
    textured.setMat4("projectionMatrix", projection);
    textured.setMat4("viewMatrix", createViewMatrix(camera));
    textured.setMat4("light_view_proj_matrix", lightSpace);
    textured.setVec3("viewPos", camera.position);
    textured.setVec3("lightPos", lightPosition);
    textured.setVec3("lightDir", glm::normalize(-lightPosition));
    glBindTexture(GL_TEXTURE_2D, shadowTexture);
    glActiveTexture(GL_TEXTURE0);
    drawGeometry(textured, false);
    drawBackdrop(textured);
    drawGuides();

    glBindVertexArray(0);
    glfwSwapBuffers(window);
}

void Scene::drawGeometry(const Shader& shader, bool shadowPass) {
    if (!shadowPass) {
        shader.setBool("drawTexture", texturedModels && !studioBackdrop);
        shader.setFloat("ambientStrength", studioBackdrop ? 0.48f : 0.3f);
        if constexpr (studioBackdrop) {
            shader.setVec3("objectColor", {0.061f, 0.085f, 0.128f});
        } else {
            shader.setVec3("objectColor", texturedModels ? glm::vec3(1.0f) : glm::vec3(0.5f, 1.0f, 0.5f));
            glBindTexture(GL_TEXTURE_2D, ground.getTextureID());
        }
    }
    const float groundHeight = studioBackdrop ? -0.02f : -0.01f;
    const auto groundTransform = glm::translate(orientation, {-5.0f, groundHeight, -5.0f});
    shader.setMat4("worldMatrix", glm::scale(groundTransform, {100.0f, 0.1f, 100.0f}));
    glDrawArrays(GL_TRIANGLES, 30, 6);

    if (!shadowPass && studioBackdrop) {
        shader.setBool("drawTexture", texturedModels);
        shader.setFloat("ambientStrength", 0.3f);
    }
    for (std::size_t i = 0; i < letters.size(); ++i) {
        const auto& pose = letters[i];
        const auto& design = letterDesigns[i];
        auto model = glm::scale(glm::translate(orientation, design.position + pose.offset) * pose.shear,
                               glm::vec3(pose.scale));
        model = glm::rotate(model, -pose.rotation.x, {1, 0, 0});
        model = glm::rotate(model, -pose.rotation.y, {0, 1, 0});
        model = glm::rotate(model, -pose.rotation.z, {0, 0, 1});
        if (!shadowPass) {
            shader.setVec3("objectColor", texturedModels ? glm::vec3(1.0f) : design.color);
        }
        for (auto part = design.begin; part != design.end; ++part) {
            glm::mat4 shear(1.0f);
            shear[1][0] = part->shear;
            auto transform = glm::translate(model, part->offset) * shear;
            transform = glm::rotate(transform, glm::radians(part->angle), {0, 0, 1});
            shader.setMat4("worldMatrix", glm::scale(transform, part->scale));
            if (!shadowPass) {
                glBindTexture(GL_TEXTURE_2D, (part->metal ? metal : box).getTextureID());
            }
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
    drawPuzzle(shader, shadowPass);
}

void Scene::drawPuzzle(const Shader& shader, bool shadowPass) {
    const auto root = glm::translate(orientation, {0.0f, 3.0f, 2.0f});
    const auto* active = puzzle.activeTurn();
    auto turningRoot = root;
    if (active) {
        glm::vec3 axis(0.0f);
        axis[active->axis] = 1.0f;
        turningRoot = glm::rotate(root, puzzle.turnAngle(), axis);
    }
    shader.setFloat("ambientStrength", 1.0f);
    for (const auto& cubie : puzzle.pieces()) {
        const auto& home = cubie.home;
        if (home == glm::ivec3(0)) {
            continue;
        }
        const auto& pieceRoot = active && active->affects(cubie.position) ? turningRoot : root;
        auto transform = glm::translate(pieceRoot, glm::vec3(cubie.position) * 0.3f);
        const glm::mat4 basis(glm::vec4(glm::vec3(cubie.axes[0]), 0.0f),
                              glm::vec4(glm::vec3(cubie.axes[1]), 0.0f),
                              glm::vec4(glm::vec3(cubie.axes[2]), 0.0f),
                              glm::vec4(0, 0, 0, 1));
        transform = glm::translate(glm::scale(transform * basis, glm::vec3(2.85f)), glm::vec3(-0.05f));
        shader.setMat4("worldMatrix", transform);
        if (shadowPass) {
            glDrawArrays(GL_TRIANGLES, 0, 36);
            continue;
        }

        // Index the original sticker artwork in the cubie's local face order.
        const int row = 3 * (1 - home.y);
        const int tile[] = {
            home.z == -1 ? row + 1 - home.x : -1,
            home.x ==  1 ? row + 1 - home.z : -1,
            home.z ==  1 ? row + 1 + home.x : -1,
            home.x == -1 ? row + 1 + home.z : -1,
            home.y == -1 ? 3 * (1 - home.z) + 1 + home.x : -1,
            home.y ==  1 ? 3 * (1 + home.z) + 1 + home.x : -1,
        };
        for (int face = 0; face < 6; ++face) {
            const bool hasSticker = tile[face] >= 0;
            shader.setBool("drawTexture", hasSticker);
            shader.setVec3("objectColor", glm::vec3(hasSticker ? 1.0f : 0.035f));
            if (hasSticker) {
                const auto& image = face == 0 ? digits[tile[face] + 1] : stickers[face][tile[face]];
                glBindTexture(GL_TEXTURE_2D, image.getTextureID());
            }
            glDrawArrays(GL_TRIANGLES, 6 * face, 6);
        }
    }
}

void Scene::drawBackdrop(const Shader& shader) {
    shader.setVec3("objectColor", glm::vec3(1.0f));
    shader.setBool("drawTexture", false);
    shader.setFloat("ambientStrength", 1.0f);
    shader.setMat4("worldMatrix", glm::scale(glm::translate(orientation, lightPosition), glm::vec3(0.25f)));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    shader.setBool("drawTexture", true);
    const GLboolean culling = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    int ticks = static_cast<int>(std::fmod((lastFrameTime - timerStart) * 10.0, 1000000.0));
    for (int i = 0; i < 6; ++i, ticks /= 10) {
        shader.setMat4("worldMatrix", glm::translate(orientation, {0.2f - 0.1f * i, 4.0f, 2.0f}));
        glBindTexture(GL_TEXTURE_2D, digits[ticks % 10].getTextureID());
        glDrawArrays(GL_TRIANGLES, 12, 6);
    }
    if (culling) glEnable(GL_CULL_FACE);

    const auto sky = glm::scale(glm::translate(identity, camera.position) * orientation, glm::vec3(50000.0f));
    shader.setMat4("worldMatrix", glm::translate(sky, glm::vec3(-0.05f)));
    glFrontFace(GL_CCW);
    if constexpr (studioBackdrop) {
        shader.setBool("drawStudioBackdrop", true);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        shader.setBool("drawStudioBackdrop", false);
    } else {
        for (int face = 0; face < 6; ++face) {
            glBindTexture(GL_TEXTURE_2D, skybox[face].getTextureID());
            glDrawArrays(GL_TRIANGLES, 6 * face, 6);
        }
    }
    glFrontFace(GL_CW);
}

void Scene::drawGuides() {
    guides.bind();
    textured.setBool("drawTexture", false);
    textured.setFloat("ambientStrength", 1.0f);
    textured.setMat4("worldMatrix", orientation);
    const glm::vec3 desktopAxes[] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    const glm::vec3 studioAxes[] = {{0.53f, 0.27f, 0.32f}, {0.22f, 0.44f, 0.36f}, {0.25f, 0.38f, 0.58f}};
    const auto& colors = studioBackdrop ? studioAxes : desktopAxes;
    for (int axis = 0; axis < 3; ++axis) {
        textured.setVec3("objectColor", colors[axis]);
        glDrawArrays(GL_LINES, 2 * axis, 2);
    }
    textured.setVec3("objectColor", studioBackdrop ? glm::vec3(0.082f, 0.114f, 0.165f) : glm::vec3(0.35f));
    glDrawArrays(GL_LINES, 6, 4 * (Mesh::gridDivisions + 1));
}
