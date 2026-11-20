#include "walker.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

}  // namespace

int main() {
    Walker walker;
    const auto flat = [](glm::vec2) { return 0.0f; };
    const auto open = [](glm::vec3) { return false; };

    walker.advance(0.5f, {5.6f, 2.0f}, false, flat, [](glm::vec3 p) { return p.x > 0.65f; });
    check(walker.position().x <= 0.65f && walker.position().x > 0.5f, "A sprint must not pass through a wall.");
    check(walker.position().z > 0.95f, "Movement along a wall must remain possible.");

    walker.reset({0, -0.35f, 0});
    const auto step = [](glm::vec2 p) { return p.x >= 0.5f ? 0.0f : -0.35f; };
    walker.advance(0.5f, {3.2f, 0}, false, step, open);
    check(walker.position().x > 0.5f && walker.position().y == 0.0f,
          "The player must be able to step onto the platform.");

    walker.reset({0, 0, 0});
    const auto ledge = [](glm::vec2 p) { return p.x >= 0.5f ? 1.4f : 0.0f; };
    walker.advance(0.5f, {3.2f, 0}, false, ledge, open);
    check(walker.position().x < 0.5f, "The player must stop at a ledge taller than a step.");

    walker.reset({0, 0, 0});
    walker.advance(0.2f, {0, 0}, true, flat, open);
    check(walker.position().y > 0.4f, "Jump must lift the player off the ground.");
    walker.advance(1.0f, {0, 0}, false, flat, open);
    check(std::abs(walker.position().y) < 0.0001f, "The player must land on the floor.");

    walker.advance(0.25f, {0, 0}, true, flat, [](glm::vec3 p) { return p.y + Walker::height > 1.95f; });
    check(walker.position().y + Walker::height <= 1.95f, "The player must not jump through overhead geometry.");
}
