#pragma once
#ifndef RENDER
#define RENDER
#include "Windowing.h"
#include "RenderBase.h"
#include <json.hpp>
#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>
#include <meshoptimizer.h>
#include <vector>
#include <Eventsystem/Eventsystem.hpp>
namespace Canella
{

    struct SphereBoundingVolume
    {
        glm::vec4 center_radius;
    };

    struct Time
    {
        Time(float t) : time(t){};
        double time_in_milli() { return time * 1000; }
        float time_in_sec() { return time; }
        float last_time_frame = 0;
        float time;
        float delta;
    };

    class Render;
    struct MeshletBound
    {

        glm::vec4 cone_apex;
        glm::vec4 cone_axis;
        glm::vec4 cone_cutoff;
        glm::vec4 center;
    };

    struct RenderCameraData
    {
        glm::mat4 view;
        glm::mat4 projection;
    };

    struct Meshlet
    {
        std::vector<meshopt_Meshlet> meshlets;
        std::vector<MeshletBound> bounds;
        std::vector<unsigned int> meshlet_vertices;
        std::vector<unsigned char> meshlet_triangles;
    };

    struct Vertex
    {
        glm::vec4 position;
    };
    struct Mesh
    {
        uint32_t vertex_offset;
        uint32_t index_offset;
        uint32_t index_count;
        uint32_t vertex_count;
        uint32_t meshlet_offset;
        uint32_t meshlet_triangle_offset;
        uint32_t meshlet_vertex_offset;
        uint32_t meshlet_count;

        Mesh() = default;
        Mesh(const Mesh &other) = default;
    };

    struct ModelMesh
    {
        std::vector<Vertex> positions;
        std::vector<glm::vec4> normal;
        std::vector<uint32_t> indices;
        std::vector<Mesh> meshes;
        glm::mat4 *model_matrix;
        Meshlet meshlet_compositions;
    };

    struct TimeQueries
    {
        std::string name;
        std::string description;
        float time;
    };
    

    glm::vec4 compute_sphere_bounding_volume(Mesh &mesh, std::vector<Vertex> &vertices);
    void load_asset_mesh(ModelMesh &mesh, const ::std::string &assetsPath, const std::string &source);
    void load_meshlet(Canella::Meshlet &canellaMeshlet, Canella::ModelMesh &mesh, int mesh_index);
    using Drawables = std::vector<ModelMesh>;

    class LoseSwapchainEvent : public Event<Canella::Render *>{};
    class RenderEvent : public Event<Canella::Render *>{};
    class DrawableEnqueueEvent : public Event<Canella::Render *>  {};


    class Render
    {
    public:
        Render() = default;
        virtual ~Render(){};
        virtual void build(nlohmann::json &json) = 0;
        virtual void enqueue_drawables(Drawables &) = 0;
        virtual void enqueue_drawable(ModelMesh& ) = 0;
        virtual void render(glm::mat4 &, glm::mat4 &) = 0;
        virtual Drawables &get_drawables() = 0;

        // Render Events`
        Canella::LoseSwapchainEvent OnLostSwapchain;
        Canella::DrawableEnqueueEvent OnEnqueueDrawable;
        Canella::RenderEvent OnRender;
    };
}

#endif