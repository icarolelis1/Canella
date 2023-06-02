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
    class Render;
    struct MeshletBound{

        glm::vec4 cone_apex;
        glm::vec4 cone_axis;
        glm::vec4 cone_cutoff;
    };

    struct Meshlet{
        std::vector<meshopt_Meshlet> meshlets;
        std::vector<MeshletBound> bounds;
        std::vector<unsigned int> meshlet_vertices;
        std::vector<unsigned char> meshlet_triangles ;
    };

    struct   Vertex {
         glm::vec4 vertex;
    };
    struct Mesh{
        std::vector<Vertex> positions;
        std::vector<glm::vec4> normal;
        std::vector<uint32_t> indices;
        Mesh() = default;
        Mesh(const Mesh& other) = default;
    };

    struct ModelMesh{
        std::vector<Mesh> meshes;
        glm::mat4* model_matrix;
    };

    struct TimeQueryData{
        std::string name;
        std::string description;
        float time;
    };

    void load_asset_mesh( ModelMesh& mesh, const::std::string& assetsPath, const std::string& source );
    void load_meshlet( Meshlet &, const Mesh & mesh );
    using Drawables = std::vector<ModelMesh>;

    //Todo Finish implementation as needed
    class LoseSwapchainEvent: public Event<Canella::Render*>{};
    class RenderEvent: public Event<Canella::Render*>{};
    class DrawableEnqueueEvent: public Event<Canella::Render*>{};

    class Render
        {
        public:
            Render() = default;
            virtual ~Render() {};
            virtual void build(nlohmann::json &json) = 0;
            virtual void enqueue_drawables(Drawables&) = 0;
            virtual void render(glm::mat4& viewProjection) = 0;
            virtual void update(float time) = 0;
            virtual void log_statistic_data(TimeQueryData&) = 0;
            virtual Drawables& get_drawables() = 0;

            //Render Events`
            Canella::LoseSwapchainEvent OnLostSwapchain;
            Canella::DrawableEnqueueEvent OnDrawableEnqueue;
            Canella::RenderEvent OnRender;
        };
    }
#endif