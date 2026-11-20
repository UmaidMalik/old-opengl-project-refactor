#include "scene.h"

#include <exception>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace {

GLFWwindow* openWindow() {
    if (!glfwInit()) {
        return nullptr;
    }
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
#ifndef __EMSCRIPTEN__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
#endif

    GLFWwindow* window = glfwCreateWindow(1024, 768, "OpenGL Letter Scene", nullptr, nullptr);
    if (!window) {
        return nullptr;
    }
    glfwMakeContextCurrent(window);

#ifndef __EMSCRIPTEN__
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        return nullptr;
    }
#endif
    return window;
}

#ifdef __EMSCRIPTEN__
struct BrowserScene {
    GLFWwindow* window;
    Scene scene;
};

void browserFrame(void* data) {
    auto* app = static_cast<BrowserScene*>(data);
    if (glfwWindowShouldClose(app->window)) {
        emscripten_cancel_main_loop();
        delete app;
        glfwTerminate();
        return;
    }
    app->scene.frame();
}
#endif

}  // namespace

int main() {
    GLFWwindow* window = openWindow();
    if (!window) {
        std::cerr << "Could not initialize the graphics window.\n";
        glfwTerminate();
        return 1;
    }

    try {
#ifdef __EMSCRIPTEN__
        auto* app = new BrowserScene{window, Scene(window)};
        emscripten_set_main_loop_arg(browserFrame, app, 0, false);
#else
        Scene scene(window);
        while (!glfwWindowShouldClose(window)) {
            scene.frame();
        }
#endif
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        glfwTerminate();
        return 1;
    }
#ifndef __EMSCRIPTEN__
    glfwTerminate();
#endif
    return 0;
}
