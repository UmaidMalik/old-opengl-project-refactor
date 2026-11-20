#pragma once

#include <array>
#include <deque>
#include <glm/glm.hpp>

class RubiksCube {
public:
    struct Cubie {
        glm::ivec3 home;
        glm::ivec3 position;
        // The columns are the cubie's local face axes in puzzle coordinates.
        std::array<glm::ivec3, 3> axes;
    };

    struct Turn {
        int axis;
        int layer;
        int direction;

        bool affects(const glm::ivec3& position) const { return position[axis] == layer; }
    };

    static constexpr float turnSeconds = 0.24f;

    RubiksCube();
    void queueTurn(Turn move);
    void advance(float elapsed);
    const Turn* activeTurn() const;
    float turnAngle() const;
    void turn(int axis, int layer, int direction);
    const std::array<Cubie, 27>& pieces() const { return cubies; }

private:
    std::array<Cubie, 27> cubies;
    std::deque<Turn> turns;
    float turnElapsed = 0.0f;
};
