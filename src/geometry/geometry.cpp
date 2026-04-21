#include "geometry.hpp"

// TODO: rework when implementing Meshes
void Geometry::cube(Buffer& buffer, const glm::vec3& size, const glm::vec3& center) {
    const glm::vec3 s = size / 2.0f;

    std::vector<float> vertices = {
        // -X
        center.x - s.x, center.y - s.y, center.z + s.z,
        center.x - s.x, center.y + s.y, center.z + s.z,
        center.x - s.x, center.y + s.y, center.z - s.z,
        center.x - s.x, center.y - s.y, center.z + s.z,
        center.x - s.x, center.y + s.y, center.z - s.z,
        center.x - s.x, center.y - s.y, center.z - s.z,

        // +X
        center.x + s.x, center.y - s.y, center.z - s.z,
        center.x + s.x, center.y + s.y, center.z - s.z,
        center.x + s.x, center.y - s.y, center.z + s.z,
        center.x + s.x, center.y + s.y, center.z - s.z,
        center.x + s.x, center.y + s.y, center.z + s.z,
        center.x + s.x, center.y - s.y, center.z + s.z,

        // -Y
        center.x + s.x, center.y - s.y, center.z - s.z,
        center.x + s.x, center.y - s.y, center.z + s.z,
        center.x - s.x, center.y - s.y, center.z + s.z,
        center.x + s.x, center.y - s.y, center.z - s.z,
        center.x - s.x, center.y - s.y, center.z + s.z,
        center.x - s.x, center.y - s.y, center.z - s.z,

        // +Y
        center.x + s.x, center.y + s.y, center.z - s.z,
        center.x - s.x, center.y + s.y, center.z - s.z,
        center.x + s.x, center.y + s.y, center.z + s.z,
        center.x - s.x, center.y + s.y, center.z - s.z,
        center.x - s.x, center.y + s.y, center.z + s.z,
        center.x + s.x, center.y + s.y, center.z + s.z,

        // -Z
        center.x + s.x, center.y + s.y, center.z - s.z,
        center.x + s.x, center.y - s.y, center.z - s.z,
        center.x - s.x, center.y - s.y, center.z - s.z,
        center.x + s.x, center.y + s.y, center.z - s.z,
        center.x - s.x, center.y - s.y, center.z - s.z,
        center.x - s.x, center.y + s.y, center.z - s.z,

        // +Z
        center.x + s.x, center.y - s.y, center.z + s.z,
        center.x + s.x, center.y + s.y, center.z + s.z,
        center.x - s.x, center.y - s.y, center.z + s.z,
        center.x + s.x, center.y + s.y, center.z + s.z,
        center.x - s.x, center.y + s.y, center.z + s.z,
        center.x - s.x, center.y - s.y, center.z + s.z,
    };

    int32_t verticesSize = vertices.size() * sizeof(float);

    buffer.create(verticesSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    buffer.upload(vertices.data(), verticesSize);
}
