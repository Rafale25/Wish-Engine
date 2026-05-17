#pragma once

#include "Buffer.hpp"
#include <glm/ext/vector_float3.hpp>

class Geometry {
    public:
        // TODO: add position and size args (https://moderngl-window.readthedocs.io/en/latest/reference/geometry.html)
        static void cube(Buffer& buffer, const glm::vec3& size, const glm::vec3& center);
        static void quadFullscreen(Buffer& bufferVertices, Buffer& bufferIndices);

        // static Buffer quad_2d();
        // void sphere() {}
        // void bbox() {}
};
