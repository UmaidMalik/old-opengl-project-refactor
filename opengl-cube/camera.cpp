#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

CameraState initialCamera()
{
#ifdef __EMSCRIPTEN__
    CameraState camera{{1.4f, 4.2f, 4.5f}, {}, {0.0f, 1.0f, 0.0f},
                       115.0f, -24.0f, 1.0f, 4.0f, 60.0f};
#else
    CameraState camera{{0.0f, 4.0f, 3.5f}, {}, {0.0f, 1.0f, 0.0f},
                       90.0f, 0.0f, 1.0f, 4.0f, 90.0f};
#endif
    updateCameraDirection(camera);
    return camera;
}

void updateCameraDirection(CameraState& camera)
{
    camera.direction = glm::vec3(
        glm::cos(glm::radians(camera.verticalAngle)) *
            glm::cos(glm::radians(camera.horizontalAngle)),
        glm::sin(glm::radians(camera.verticalAngle)),
        -glm::cos(glm::radians(camera.verticalAngle)) *
            glm::sin(glm::radians(camera.horizontalAngle))
    );
}

glm::mat4 createViewMatrix(const CameraState& camera)
{
    return glm::lookAt(
        camera.position,
        camera.position + camera.direction,
        camera.up
    );
}

glm::mat4 createProjectionMatrix(const CameraState& camera, float aspect, bool orthographic)
{
    if (orthographic) {
        return glm::ortho(-3.0f * aspect, 3.0f * aspect, -3.0f, 3.0f, -500.0f, 500000.0f);
    }
    return glm::perspective(glm::radians(camera.fieldOfView), aspect, 0.05f, 10000.0f);
}
