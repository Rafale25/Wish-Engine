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

void Geometry::quadFullscreen(Buffer& bufferVertices, Buffer& bufferIndices) {
    const VkDeviceSize indexCount{6};
    std::vector<float> vertices {
        -1,  1, 0,   0, 0,
         1,  1, 0,   1, 0,
        -1, -1, 0,   0, 1,
         1, -1, 0,   1, 1,
    };
    std::vector<uint16_t> indices{
        0, 1, 2,
        2, 1, 3,
    };

    int verticesSize = vertices.size() * sizeof(float);
    int indicesSize = indices.size() * sizeof(uint16_t);

    bufferVertices.create(verticesSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    bufferIndices.create(indicesSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    bufferVertices.upload(vertices.data(), verticesSize);
    bufferIndices.upload(indices.data(), indicesSize);
}
