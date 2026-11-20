#pragma once

#include <glm/glm.hpp>

struct LetterPart {
    glm::vec3 offset;
    glm::vec3 scale;
    bool metal;
    float shear = 0.0f;
    float angle = 0.0f;
};

struct LetterDesign {
    glm::vec3 position;
    glm::vec3 color;
    const LetterPart* begin;
    const LetterPart* end;
};

extern const LetterDesign letterDesigns[5];
