#include "rubiks_cube.h"

namespace {

void rotateQuarter(glm::ivec3& vector, int axis, int direction) {
    const int first = (axis + 1) % 3;
    const int second = (axis + 2) % 3;
    const int oldFirst = vector[first];
    vector[first] = -direction * vector[second];
    vector[second] = direction * oldFirst;
}

}  // namespace

RubiksCube::RubiksCube() {
    auto piece = cubies.begin();
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            for (int z = -1; z <= 1; ++z) {
                const glm::ivec3 position(x, y, z);
                *piece++ = {position, position, {{{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}}};
            }
        }
    }
}

void RubiksCube::queueTurn(Turn move) {
    turns.push_back(move);
}

void RubiksCube::advance(float elapsed) {
    turnElapsed += elapsed;
    while (!turns.empty() && turnElapsed >= turnSeconds) {
        const auto move = turns.front();
        turn(move.axis, move.layer, move.direction);
        turns.pop_front();
        turnElapsed -= turnSeconds;
    }
    if (turns.empty()) {
        turnElapsed = 0.0f;
    }
}

const RubiksCube::Turn* RubiksCube::activeTurn() const {
    return turns.empty() ? nullptr : &turns.front();
}

float RubiksCube::turnAngle() const {
    const auto* move = activeTurn();
    if (!move) {
        return 0.0f;
    }
    const float t = turnElapsed / turnSeconds;
    return move->direction * glm::radians(90.0f) * glm::smoothstep(0.0f, 1.0f, t);
}

void RubiksCube::turn(int axis, int layer, int direction) {
    const Turn move{axis, layer, direction};
    for (auto& cubie : cubies) {
        if (!move.affects(cubie.position)) {
            continue;
        }
        rotateQuarter(cubie.position, axis, direction);
        for (auto& localAxis : cubie.axes) {
            rotateQuarter(localAxis, axis, direction);
        }
    }
}
