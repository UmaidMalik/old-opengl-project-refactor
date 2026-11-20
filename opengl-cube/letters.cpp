#include "letters.h"

#include <iterator>

namespace {

const LetterPart partsA7[] = {
    {{-0.5f, -0.2f, 0.0f}, {1.0f, 5.0f, 1.0f}, false, 0.35f},
    {{-0.1f, -0.2f, 0.0f}, {1.0f, 5.0f, 1.0f}, false, -0.35f},
    {{-0.35f, -0.05f, 0.0f}, {2.0f, 0.75f, 1.0f}, false},
    {{0.2f, -0.2f, 0.0f}, {1.0f, 4.0f, 1.0f}, true, 0.3f},
    {{0.12f, 0.2f, 0.0f}, {3.0f, 1.0f, 1.0f}, true},
};

const LetterPart partsO9[] = {
    {{-0.5f, -0.2f, 0.0f}, {1.0f, 5.0f, 1.0f}, false},
    {{-0.1f, -0.2f, 0.0f}, {1.0f, 5.0f, 1.0f}, false},
    {{-0.4f, -0.2f, 0.0f}, {3.0f, 0.75f, 1.0f}, false},
    {{-0.4f, 0.225f, 0.0f}, {3.0f, 0.75f, 1.0f}, false},
    {{0.12f, 0.225f, 0.0f}, {4.0f, 0.75f, 1.0f}, true},
    {{0.12f, -0.025f, 0.0f}, {3.0f, 0.75f, 1.0f}, true},
    {{0.12f, -0.2f, 0.0f}, {3.0f, 0.75f, 1.0f}, true},
    {{0.42f, -0.2f, 0.0f}, {1.0f, 5.0f, 1.0f}, true},
    {{0.12f, 0.05f, 0.0f}, {0.75f, 1.8f, 1.0f}, true},
};

const LetterPart partsS0[] = {
    {{-0.5f, 0.3f, 0.0f}, {4.0f, 1.0f, 1.0f}, false},
    {{-0.6f, 0.1f, 0.0f}, {1.0f, 2.0f, 1.0f}, false},
    {{-0.5f, 0.0f, 0.0f}, {3.0f, 1.0f, 1.0f}, false},
    {{-0.2f, -0.2f, 0.0f}, {1.0f, 2.0f, 1.0f}, false},
    {{-0.6f, -0.3f, 0.0f}, {4.0f, 1.0f, 1.0f}, false},
    {{0.1f, -0.2f, 0.0f}, {1.0f, 5.0f, 1.0f}, true},
    {{0.5f, -0.2f, 0.0f}, {1.0f, 5.0f, 1.0f}, true},
    {{0.2f, 0.3f, 0.0f}, {3.0f, 1.0f, 1.0f}, true},
    {{0.2f, -0.3f, 0.0f}, {3.0f, 1.0f, 1.0f}, true},
};

const LetterPart partsM6[] = {
    {{-0.5f, -0.2f, 0.0f}, {1.0f, 5.0f, 1.0f}, false},
    {{-0.3f, -0.2f, 0.0f}, {1.0f, 5.0f, 1.0f}, false, 0.35f},
    {{-0.3f, -0.2f, 0.0f}, {1.0f, 5.0f, 1.0f}, false, -0.35f},
    {{-0.1f, -0.2f, 0.0f}, {1.0f, 5.0f, 1.0f}, false},
    {{0.3f, -0.2f, 0.0f}, {1.0f, 3.0f, 1.0f}, true},
    {{0.1f, -0.2f, 0.0f}, {1.0f, 5.0f, 1.0f}, true},
    {{0.1f, -0.2f, 0.0f}, {2.0f, 1.0f, 1.0f}, true},
    {{0.1f, 0.2f, 0.0f}, {3.0f, 1.0f, 1.0f}, true},
};

const LetterPart partsN7[] = {
    {{-0.5f, 0.0f, 0.0f}, {1.0f, 5.0f, 1.0f}, false},
    {{-0.15f, 0.0f, 0.0f}, {1.0f, 5.0f, 1.0f}, false},
    {{-0.15f, 0.0f, 0.0f}, {1.0f, 5.0f, 1.0f}, false, -0.7f},
    {{0.1f, 0.0f, 0.0f}, {1.0f, 5.0f, 1.0f}, true, 0.5f},
    {{0.35f, 0.4f, 0.0f}, {1.0f, 2.5f, 1.0f}, true, 0.0f, 90.0f},
};

}  // namespace

const LetterDesign letterDesigns[5] = {
    {{-4.0f, 0.5f, -4.0f}, {1.0f, 1.0f, 0.0f}, std::begin(partsA7), std::end(partsA7)},
    {{4.0f, 0.5f, -4.0f}, {1.0f, 0.6f, 0.9f}, std::begin(partsO9), std::end(partsO9)},
    {{-4.0f, 0.5f, 4.0f}, {0.0f, 0.0f, 0.5f}, std::begin(partsS0), std::end(partsS0)},
    {{4.0f, 0.5f, 4.0f}, {1.0f, 0.0f, 0.0f}, std::begin(partsM6), std::end(partsM6)},
    {{0.0f, 0.3f, -4.0f}, {0.0f, 1.0f, 1.0f}, std::begin(partsN7), std::end(partsN7)},
};
