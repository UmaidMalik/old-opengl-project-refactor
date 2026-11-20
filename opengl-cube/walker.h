#pragma once

#include <functional>
#include <glm/glm.hpp>

class Walker {
public:
    static constexpr float radius = 0.25f;
    static constexpr float height = 1.72f;
    static constexpr float eyeHeight = 1.61f;

    void reset(glm::vec3 position);
    void advance(float elapsed, glm::vec2 velocity, bool jump,
                 const std::function<float(glm::vec2)>& floor,
                 const std::function<bool(glm::vec3)>& blocked);
    glm::vec3 eye() const { return feet + glm::vec3(0, eyeHeight, 0); }
    glm::vec3 position() const { return feet; }

private:
    glm::vec3 feet{0.0f};
    float verticalSpeed = 0;
    bool grounded = true;
};
