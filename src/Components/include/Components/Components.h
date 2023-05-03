#ifndef CANELLA_COMPONENTS
#define CANELLA_COMPONENTS
/*
 * List of Basic Components
 */
#include "entt.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "json.hpp"
#include <map>
#include "Render/Render.h"

namespace Canella
{
    struct Euler
    {
        glm::vec3 front;
        glm::vec3 up;
        glm::vec3 right;
    };

    struct HierarchyComponent
    {
    };

    class TransformComponent  {
    public:
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
        TransformComponent(const TransformComponent&)=default;
        TransformComponent() = default;
        
    };

    struct CameraComponent
    {
        Euler euler;
        glm::vec3 position;
        glm::mat4 projection;
        glm::vec3 target;
        float zNear;
        float zFar;
    };

    struct CameraControllerComponent
    {
        float velocity = .04f;
        float YAW = 90;
        float PITCH = 0;
        float ROLL;
        CameraComponent* camera_component;
    };

    struct MeshAssetComponent
    {
        Mesh mesh;
        std::string source;
        bool isStatic = false;
        MeshAssetComponent() = default;
    };

    void SerializeTransform(nlohmann::json& data,entt::registry& registry,entt::entity entity);
    void SerializeCamera(nlohmann::json& data,entt::registry& registry,entt::entity entity);
    void SerializeMeshAsset(nlohmann::json& data,entt::registry& registry,entt::entity entity);
    void DeserializeTransform(nlohmann::json& data,TransformComponent&);
    void DeserializeCamera(nlohmann::json& data,CameraComponent&);
    void DeserializeMeshAsset(nlohmann::json& data,MeshAssetComponent&);
    
    
}

#endif