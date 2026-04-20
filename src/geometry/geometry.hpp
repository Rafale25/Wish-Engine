#pragma once

#include "Buffer.hpp"
#include <glm/ext/vector_float3.hpp>

class Geometry {
    public:
        // TODO: add position and size args (https://moderngl-window.readthedocs.io/en/latest/reference/geometry.html)
        // static Buffer quad_2d();
        static void cube(Buffer& buffer, const glm::vec3& size, const glm::vec3& center);

        // void quad_fs() {}
        // void sphere() {}
        // void bbox() {}
};
