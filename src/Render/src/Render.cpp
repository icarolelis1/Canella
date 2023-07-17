#include "Render/Render.h"
#include <Logger/Logger.hpp>


glm::vec4 Canella::compute_sphere_bounding_volume(Mesh &mesh, std::vector<Vertex> &vertices)
{
    std::vector<Vertex> slice = std::vector<Vertex>(vertices.begin() + mesh.vertex_offset, vertices.begin() + mesh.vertex_count + mesh.vertex_offset);

    glm::vec3 center = glm::vec3(0);

    for (auto &vertex : slice)
        center += glm::vec3(vertex.position.x, vertex.position.y, vertex.position.z);
    center /= slice.size();

    double radius = 0.0f;
    for (auto &v : slice)
        radius = max(radius, glm::distance(center, glm::vec3(v.position.x, v.position.y, v.position.z)));

    return glm::vec4(center.x, center.y, center.z, radius);
}
