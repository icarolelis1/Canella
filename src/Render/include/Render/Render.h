#pragma once
#ifndef RENDER
#define RENDER
#include "Windowing.h"
#include "RenderBase.h"
#include <json.hpp>
#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>
#include <Meshoptimizer/meshoptimizer.h>
#include <vector>
namespace Canella
{
    struct Meshlet{
        std::vector<meshopt_Meshlet> meshlets;
        std::vector<meshopt_Bounds> bounds;
    };

    struct Mesh{
        std::vector<glm::vec4> positions;
        std::vector<glm::vec4> normal;
        std::vector<int8_t> indices;
        Mesh() = default;
        Mesh(const Mesh& other) = default;
    };

    struct ModelMesh{
        std::vector<Mesh> meshes;
        glm::mat4* model_matrix;
    };


    void load_asset_mesh( ModelMesh& mesh, const::std::string& assetsPath, const std::string& source );
    void load_meshlet( Meshlet &, const Mesh & mesh );
    using Drawables = std::vector<ModelMesh>;
    
    class Render
        {
        public:
            Render() = default;
            explicit Render(nlohmann::json &json);
            virtual void enqueue_drawables(Drawables&) = 0;
            virtual Drawables& get_drawables() = 0;
            virtual void render(glm::mat4& viewProjection) = 0;
            virtual void update(float time) = 0;
        };
    }
#endif