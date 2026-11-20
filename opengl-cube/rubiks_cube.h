#pragma once

#include <array>
#include <glm/glm.hpp>

class RubiksCube {
public:
    struct Cubie {
        glm::ivec3 home;
        glm::ivec3 position;
        // The columns are the cubie's local face axes in puzzle coordinates.
        std::array<glm::ivec3, 3> axes;
    };

    RubiksCube();
    void turn(int axis, int layer, int direction);
    const std::array<Cubie, 27>& pieces() const { return cubies; }

private:
    std::array<Cubie, 27> cubies;
};
