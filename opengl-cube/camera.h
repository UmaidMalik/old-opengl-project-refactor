#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

struct CameraState
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;

    float horizontalAngle;
    float verticalAngle;
    float normalSpeed;
    float fastSpeed;
    float fieldOfView;
};

void updateCameraDirection(CameraState& camera);
glm::mat4 createViewMatrix(const CameraState& camera);
glm::mat4 createProjectionMatrix(int width, int height);