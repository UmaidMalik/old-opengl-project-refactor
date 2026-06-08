#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

glm::mat4 createProjectionMatrix(
    int width,
    int height
)
{
	return glm::perspective(
		90.0f,
		(float)width / (float)height,
		0.0005f,
		500000.0f
	);
}