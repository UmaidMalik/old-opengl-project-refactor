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

void RubiksCube::turn(int axis, int layer, int direction) {
    for (auto& cubie : cubies) {
        if (cubie.position[axis] != layer) {
            continue;
        }
        rotateQuarter(cubie.position, axis, direction);
        for (auto& localAxis : cubie.axes) {
            rotateQuarter(localAxis, axis, direction);
        }
    }
}
