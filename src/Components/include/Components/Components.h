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
#include "CanellaUtility/CanellaUtility.h"
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
        glm::mat4 modelMatrix;
        TransformComponent(const TransformComponent&)=default;
        TransformComponent() = default;
        
    };

    struct CameraComponent
    {
        Euler euler;
        glm::vec3 position;
        glm::mat4 projection;
        glm::vec3 target;
        float fovy;
        float zNear;
        float zFar;
        glm::mat4 viewProjection;
    };

    struct CameraControllerComponent
    {
        float velocity = .04f;
        float YAW = 90;
        float PITCH = 0;
        float ROLL;
        CameraComponent* camera_component;
    };

    struct ModelAssetComponent
    {
        ModelMesh mesh;
        std::string source;
        bool isStatic = false;
        ModelAssetComponent() = default;
    };

    class ScriptableEntity{

    };

    struct ScriptTest : public ScriptableEntity
    {
        void instantiate(){}
        void destroy(){}
        void on_create(){
            Canella::Logger::Error("CALLING ON CREATE BITCH")
            ;}
        void on_update(float t){ Canella::Logger::Info("SCRIPT IS WORKING");}
        void on_destroy(){ Canella::Logger::Info("DESTROING");}
    };

    struct Behavior{
        std::function<void()> instantiate;
        std::function<void(ScriptableEntity* entit)> on_create;
        std::function<void(ScriptableEntity*,float)> on_update;
        std::function<void(ScriptableEntity* entit)> on_destroy;
        std::function<void()> destroy;

        ScriptableEntity * entity;
        template<typename T>
        void bind(){

            instantiate = [&](){ entity = new T();};
            destroy = [&](){ delete (T*)entity; entity = nullptr; };
            on_create = [](ScriptableEntity* entity) {
                ((T*)entity)->on_create();
            };
            on_update = [](ScriptableEntity* entity,float t) {((T*)entity)->on_update(t);};
            on_destroy = [](ScriptableEntity* entity) {((T*)entity)->on_destroy();};
        }
    };

    void SerializeTransform(nlohmann::json& data,entt::registry& registry,entt::entity entity);
    void SerializeCamera(nlohmann::json& data,entt::registry& registry,entt::entity entity);
    void SerializeMeshAsset(nlohmann::json& data,entt::registry& registry,entt::entity entity);
    void DeserializeTransform(nlohmann::json& data,TransformComponent&);
    void DeserializeCamera(nlohmann::json& data,CameraComponent&);
    void DeserializeMeshAsset(nlohmann::json& data,ModelAssetComponent&);
}

#endif