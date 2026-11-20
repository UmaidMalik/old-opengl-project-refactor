#include "rubiks_cube.h"

#include <cstdlib>
#include <iostream>
#include <set>
#include <tuple>

namespace {

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

bool sameState(const RubiksCube& a, const RubiksCube& b) {
    for (std::size_t i = 0; i < a.pieces().size(); ++i) {
        const auto& first = a.pieces()[i];
        const auto& second = b.pieces()[i];
        if (first.position != second.position || first.axes != second.axes) {
            return false;
        }
    }
    return true;
}

}  // namespace

int main() {
    const RubiksCube solved;
    for (int axis = 0; axis < 3; ++axis) {
        for (int layer = -1; layer <= 1; ++layer) {
            auto cube = solved;
            cube.turn(axis, layer, 1);
            int affected = 0;
            for (const auto& cubie : cube.pieces()) {
                const bool rotated = cubie.axes != solved.pieces().front().axes;
                check(rotated == (cubie.home[axis] == layer), "Turn changed the wrong layer.");
                affected += rotated;
            }
            check(affected == 9, "Each turn must rotate exactly nine pieces.");
            cube.turn(axis, layer, -1);
            check(sameState(cube, solved), "A turn and its inverse must cancel.");
            for (int turn = 0; turn < 4; ++turn) cube.turn(axis, layer, 1);
            check(sameState(cube, solved), "Four quarter-turns must restore the solved state.");
        }
    }

    auto cube = solved;
    cube.turn(0, 1, 1);
    cube.turn(1, -1, 1);

    std::set<std::tuple<int, int, int>> positions;
    for (const auto& cubie : cube.pieces()) {
        const auto p = cubie.position;
        check(glm::all(glm::lessThanEqual(glm::abs(p), glm::ivec3(1))), "Piece left the puzzle grid.");
        positions.emplace(p.x, p.y, p.z);
        if (cubie.home == glm::ivec3(1, 1, 1)) {
            check(p == glm::ivec3(1, -1, -1), "Second turn must select pieces at their current positions.");
            check(cubie.axes[0] == glm::ivec3(0, 0, -1) &&
                  cubie.axes[1] == glm::ivec3(1, 0, 0) &&
                  cubie.axes[2] == glm::ivec3(0, -1, 0), "Sticker orientation must follow the piece.");
        }
    }
    check(positions.size() == 27, "Cubies must occupy distinct positions.");
    cube.turn(1, -1, -1);
    cube.turn(0, 1, -1);
    check(sameState(cube, solved), "Reversing a sequence must restore the full cube state.");
}
