#include "scene.h"

#include <exception>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void, setBrowserSceneReady, (int ready), {
    Module[ready ? 'sceneReady' : 'sceneFailed']();
});
#endif

namespace {

void reportStartupError(const char* message) {
    std::cerr << message << '\n';
#ifdef __EMSCRIPTEN__
    setBrowserSceneReady(false);
#endif
}

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

BrowserScene* browserApp = nullptr;

void browserFrame(void* data) {
    auto* app = static_cast<BrowserScene*>(data);
    if (glfwWindowShouldClose(app->window)) {
        emscripten_cancel_main_loop();
        browserApp = nullptr;
        delete app;
        glfwTerminate();
        return;
    }
    app->scene.frame();
}
#endif

}  // namespace

#ifdef __EMSCRIPTEN__
extern "C" EMSCRIPTEN_KEEPALIVE void turn_cube(int key) {
    if (browserApp) browserApp->scene.queuePuzzleKey(key);
}

extern "C" EMSCRIPTEN_KEEPALIVE int toggle_walk() {
    return browserApp && browserApp->scene.toggleWalking();
}

extern "C" EMSCRIPTEN_KEEPALIVE void walk_look(float dx, float dy) {
    if (browserApp) browserApp->scene.walkLook(dx, dy);
}

extern "C" EMSCRIPTEN_KEEPALIVE void resize_scene(int width, int height) {
    if (browserApp && width > 0 && height > 0) {
        glfwSetWindowSize(browserApp->window, width, height);
    }
}
#endif

int main() {
    GLFWwindow* window = openWindow();
    if (!window) {
        reportStartupError("Could not initialize the graphics window.");
        glfwTerminate();
        return 1;
    }

    try {
#ifdef __EMSCRIPTEN__
        auto* app = new BrowserScene{window, Scene(window)};
        browserApp = app;
        emscripten_set_main_loop_arg(browserFrame, app, 0, false);
        setBrowserSceneReady(true);
#else
        Scene scene(window);
        while (!glfwWindowShouldClose(window)) {
            scene.frame();
        }
#endif
    } catch (const std::exception& error) {
        reportStartupError(error.what());
        glfwTerminate();
        return 1;
    }
#ifndef __EMSCRIPTEN__
    glfwTerminate();
#endif
    return 0;
}
