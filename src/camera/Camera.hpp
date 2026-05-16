#pragma once

#include <glm/detail/type_mat4x4.hpp>

class Camera {
    public:
        Camera() {}
        Camera(float fov, float aspect_ratio, float nearPlane, float farPlane):
            fov(fov), aspectRatio(aspect_ratio), nearPlane(nearPlane), farPlane(farPlane)
        {}

        virtual ~Camera() = default;

        virtual glm::mat4 getProjection() const = 0;
        virtual glm::mat4 getView() const = 0;
        virtual glm::vec3 getPosition() const = 0;
        virtual glm::vec3 right() const = 0;
        virtual glm::vec3 up() const = 0;
        virtual glm::vec3 forward() const = 0;

    public:
        float fov = 60.0f;
        float aspectRatio = 16.0f / 9.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
};
