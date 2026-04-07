#pragma once

#include "Camera.hpp"

struct CameraFpsCreateInfo {
    float aspect_ratio = 16.0f/9.0f;
    float fov = 60.0f;
    float yaw = 0.0f;
    float pitch = 0.0f;
    const glm::vec3& position = {0.0f, 0.0f, 0.0f};
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    bool reversedZ_enabled = false;
    bool isScreenYInverted = false;
};

class FPSCamera: public Camera {
public:
    FPSCamera() {}
    FPSCamera(CameraFpsCreateInfo ci):
        Camera(ci.fov, ci.aspect_ratio, ci.nearPlane, ci.farPlane, ci.reversedZ_enabled),
        m_position(ci.position),
        m_yaw(ci.yaw),
        m_pitch(ci.pitch),
        m_isScreenYInverted(ci.isScreenYInverted)
    {
        m_smoothPosition = ci.position;
        m_smoothYaw = ci.yaw;
        m_smoothPitch = ci.pitch;
    }

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
    float m_speed = 10.0f;
    float m_mouseSensitivity = 0.002f;

    glm::vec3 m_movement = {0.0f, 0.0f, 0.0f}; // reset each frame

    glm::vec3 m_position = {0.0f, 0.0f, 0.0f};
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
    float m_roll = 0.0f;

    float m_smoothYaw = 0.0f;
    float m_smoothPitch = 0.0f;
    float m_smoothRoll = 0.0f;
    glm::vec3 m_smoothPosition = {0.0f, 0.0f, 0.0f};

    glm::vec3 m_worldUp = {0.0, 1.0, 0.0};
    glm::vec3 m_up = {0.0, 1.0, 0.0};
    glm::vec3 m_right = {1.0, 0.0, 0.0};
    glm::vec3 m_forward = {0.0, 0.0, 1.0};

    bool m_isScreenYInverted = false;
};
