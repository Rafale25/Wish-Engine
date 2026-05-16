#include "Camera2D.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>

glm::mat4 Camera2D::getProjection() const {
    auto proj = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    return proj;
}

glm::mat4 Camera2D::getView() const {
    return glm::translate(glm::mat4(1.0f), m_position);
}

glm::vec3 Camera2D::right() const {
    return glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 Camera2D::up() const {
    return glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 Camera2D::forward() const {
    return glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 Camera2D::getPosition() const {
    return m_position;
}

void Camera2D::setPosition(const glm::vec3& p) {
    m_position = p;
}
