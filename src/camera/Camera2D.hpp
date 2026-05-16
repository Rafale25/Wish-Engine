#pragma once

#include "Camera.hpp"

class Camera2D: public Camera {
public:
    Camera2D() {}
    Camera2D(glm::vec2 center):
        Camera(60.0f, 1.0f, -1.0f, 1.0f),
        m_position(glm::vec3(center, 0.0f))
    {
    }

    glm::mat4 getProjection() const;
    glm::mat4 getView() const;
    glm::vec3 right() const;
    glm::vec3 up() const;
    glm::vec3 forward() const;

    glm::vec3 getPosition() const;
    void setPosition(const glm::vec3& p);

private:
    glm::vec3 m_position = {0.0f, 0.0f, 0.0f};
};
