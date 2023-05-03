#pragma once
#ifndef RENDER
#define RENDER
#include "Windowing.h"
#include "RenderBase.h"
#include <json.hpp>
#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>
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
    };

    struct Mesh{
        std::vector<MeshData> meshes;
    };

   
    static void LoadAssetMesh(Mesh&,const::std::string& assetsPath,const std::string& source );
    
    class Render : public RenderBase
        {
        public:
            Render() = default;
            Render(nlohmann::json &json);
            virtual void render() = 0;
            virtual void update(float time) = 0;
        };
    }
#endif