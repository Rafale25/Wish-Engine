#pragma once

#include "Camera.hpp"

class Camera2D: public Camera {
public:
    Camera2D() {}
    Camera2D(glm::vec2 center):
        Camera(60.0f, 1.0f, -1.0f, 1.0f),
        m_position(center)
    {
        m_smoothPosition = center;
    }

    glm::mat4 getProjection() const;
    glm::mat4 getView() const;

    glm::vec3 right() const;
    glm::vec3 up() const;
    glm::vec3 forward() const;

    float getYaw() const;
    float getPitch() const;
    glm::vec3 getPosition() const;
    void setPosition(const glm::vec3& p);
    void update(float dt);
    void move(const glm::vec3& direction);
    void onMouseMotion(int x, int y, int dx, int dy);
    void setSpeed(float value);

private:
    void _updateVectors();

private:
    glm::vec2 m_position = {0.0f, 0.0f};
    glm::vec2 m_smoothPosition = {0.0f, 0.0f};

    // glm::vec3 m_worldUp = {0.0, 1.0, 0.0};
    // glm::vec3 m_up = {0.0, 1.0, 0.0};
    // glm::vec3 m_right = {1.0, 0.0, 0.0};
    // glm::vec3 m_forward = {0.0, 0.0, 1.0};
};
