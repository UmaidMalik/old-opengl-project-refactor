#include "walker.h"

#include <algorithm>

void Walker::reset(glm::vec3 position) {
    feet = position;
    verticalSpeed = 0;
    grounded = true;
}

void Walker::advance(float elapsed, glm::vec2 velocity, bool jump,
                     const std::function<float(glm::vec2)>& floor,
                     const std::function<bool(glm::vec3)>& blocked) {
    constexpr float stepHeight = 0.39f;
    if (jump && grounded) {
        verticalSpeed = 4.0f;
        grounded = false;
    }

    // Small substeps keep a sprint or a dropped frame from tunneling through a trunk.
    for (float remaining = elapsed; remaining > 0; ) {
        const float dt = std::min(remaining, 1.0f / 120.0f);
        remaining -= dt;
        const auto displacement = velocity * dt;
        for (int axis = 0; axis < 2; ++axis) {
            auto next = feet;
            next[axis * 2] += displacement[axis];
            const float ground = floor({next.x, next.z});
            const float allowedRise = grounded ? stepHeight : 0.025f;
            if (ground > next.y + allowedRise) continue;
            next.y = std::max(ground, next.y);
            if (!blocked(next)) feet = next;
        }

        const float ground = floor({feet.x, feet.z});
        verticalSpeed -= 10.5f * dt;
        auto next = feet;
        next.y = std::max(ground, next.y + verticalSpeed * dt);
        if (blocked(next)) {
            grounded = verticalSpeed <= 0.0f;
            verticalSpeed = 0.0f;
        } else {
            feet = next;
            grounded = feet.y <= ground;
            if (grounded) verticalSpeed = 0.0f;
        }
    }
}
