#ifndef CANELLA_COMPONENTS
#define CANELLA_COMPONENTS
#include "Entity.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "json.hpp"
#include <map>
#include "Render/Render.h"
#include "CanellaUtility/CanellaUtility.h"

namespace Canella
{
    /*
    * List of Basic Components
    */

    struct Euler
    {
        glm::vec3 front;
        glm::vec3 up;
        glm::vec3 right;
    };

    struct HierarchyComponent{};

    struct TransformComponent  {
        TransformComponent(const TransformComponent&)=default;
        TransformComponent() = default;
        glm::vec3 position = glm::vec3(0);
        glm::vec3 rotation;
        glm::vec3 scale;
        glm::mat4 modelMatrix;
    };

    struct CameraComponent
    {
        CameraComponent() = default;
        Euler euler;
        glm::vec3 position = glm::vec3(0,3,-3);
        glm::mat4 projection;
        glm::vec3 target;
        float fovy;
        float zNear;
        float zFar;
        glm::mat4 viewProjection;
    };

    class CameraControllerComponent
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
    public:
        entt::entity entt_entity;
        virtual ~ScriptableEntity(){}
        virtual void on_create() {};
        virtual void on_destroy(){};
        virtual void on_update(float) {};
        virtual void on_start() {};
        virtual void serialize(nlohmann::json& config)  {};
        virtual void deserialize(nlohmann::json& config)  {};
        virtual void destroy(){};
    };

    class Behavior{
    public:
        ScriptableEntity* instance =  nullptr;
        ScriptableEntity*(*instantiate_fn)();
        void (*destroy_fn)(Behavior*) ;

        template<typename T>
        void bind()
        {
            instantiate_fn = []() { return static_cast<ScriptableEntity*>(new T()); };
            destroy_fn = [](Behavior* behavior) {delete behavior->instance;behavior->instance = nullptr;};
            instance = instantiate_fn();
        }
        friend class Application;
    };

    /*
    struct Behavior{
        std::function<void()> instantiate;
        std::function<void(ScriptableEntity*)> on_create;
        std::function<void(ScriptableEntity*,float)> on_update;
        std::function<void(ScriptableEntity*)> on_destroy;
        std::function<void()> serialize;
        std::function<void(nlohmann::json&)> deserialize;
        std::function<void()> destroy;
        ScriptableEntity * entity;
        template<typename T>
        void bind(){

            instantiate = [&](){ entity = new T();};
            destroy = [&](){ delete (T*)entity; entity = nullptr; };
            on_create = [](ScriptableEntity* entity) {((T*)entity)->on_create();};
            on_update = [](ScriptableEntity* entity,float t) {((T*)entity)->on_update(t);};
            on_destroy = [](ScriptableEntity* entity) {((T*)entity)->on_destroy();};
        }
    };*/

    void SerializeTransform(nlohmann::json& data,entt::registry& registry,entt::entity entity);
    void SerializeCamera(nlohmann::json& data,entt::registry& registry,entt::entity entity);
    void SerializeMeshAsset(nlohmann::json& data,entt::registry& registry,entt::entity entity);
    void SerializeCameraEditor(nlohmann::json& data,entt::registry& registry,entt::entity entity);
    void DeserializeTransform(nlohmann::json& data,TransformComponent&);
    void DeserializeCamera(nlohmann::json& data,CameraComponent&);
    void DeserializeMeshAsset(nlohmann::json& data,ModelAssetComponent&);
    void DeserializeCameraEditor(nlohmann::json& data,ModelAssetComponent&);
}

#endif