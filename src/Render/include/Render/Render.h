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

    struct BoundingSphere
    {
        alignas(16) glm::vec3 center;
        alignas(4) float radius;
    };

    struct BoundingCone
    {
        alignas(16) glm::vec3 normal;
        alignas(4) float angle;

        static BoundingCone GetApproximateReflexBoundingCone(
            std::vector<glm::vec4 *> &normals);
    };

    struct MeshletDescriptor
    {
        BoundingSphere sphere;
        BoundingCone cone;
   };


    struct MeshData{
        std::vector<glm::vec4> positions;
        std::vector<glm::vec4> normal;
        std::vector<int8_t> indices;
        std::vector<meshopt_Meshlet> meshlets;
        std::vector<meshopt_Bounds> bounds_perMeshlet;
        MeshData() = default;
        MeshData(const MeshData& other) = default;
    };

    struct Mesh{
        std::vector<MeshData> meshes;
        glm::mat4* modelMatrix;
    };

   
    void LoadAssetMesh(Mesh& mesh,const::std::string& assetsPath, const std::string& source );
    using Drawables = std::vector<Mesh>;
    
    class Render 
        {
        public:
            Render() = default;
            Render(nlohmann::json &json);
            virtual void enqueue_drawables(Drawables&) = 0;
            virtual void render(glm::mat4& viewProjection) = 0;
            virtual void update(float time) = 0;
        };
    }
#endif