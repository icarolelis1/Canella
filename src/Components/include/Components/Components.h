#ifndef CANELLA_COMPONENTS
#define CANELLA_COMPONENTS
#include "Entity.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "json.hpp"
#include <map>
#include "Render/Render.h"
#include "CanellaUtility/CanellaUtility.h"
#include "Components/ComponentTable.hpp"

namespace Canella
{
    struct Euler
    {
        glm::vec3 front;
        glm::vec3 up;
        glm::vec3 right;
    };

    struct Reference
    {
        ComponentType type;
        uint64_t uid = 0;
    };


    struct TransformComponent
    {
        TransformComponent(const TransformComponent &) = default;
        TransformComponent(){ orientation = glm::angleAxis(0.0f,glm::vec3(1,0,0));}
        Canella::Entity* owner;
        glm::vec3 position = glm::vec3(0);
        glm::vec3 rotation;
        glm::quat orientation;
        glm::vec3                     scale = glm::vec3(1);
        glm::mat4                     model_matrix;
        Reference reference;
        std::list<TransformComponent*> children;
        TransformComponent *parent = nullptr;
    };

    struct CameraComponent
    {
        CameraComponent() = default;
        ~CameraComponent() = default;
        Euler euler;
        glm::mat4 projection;
        Entity* entity;
        TransformComponent* entity_transform;
        glm::mat4 view;
        float yaw;
        float pitch;
        float fovy;
        float zNear;
        float zFar;
    };

    struct ModelAssetComponent
    {
        ModelMesh mesh;
        std::string source;
        std::string instance_src;
        std::string material_name = "DefaultMaterial";
        bool isStatic = true;
        int instance_count;
        ModelAssetComponent() = default;
    };

    class ScriptableEntity
    {
    public:
        entt::entity entt_entity;
        virtual ~ScriptableEntity() {}
        virtual void on_create(){};
        virtual void on_destroy(){};
        virtual void on_update(float){};
        virtual void on_start(){};
        virtual void serialize(nlohmann::json &config){};
        virtual void deserialize(nlohmann::json &config){};
        virtual void destroy(){};
        
    };

    class Behavior
    {
    public:
        ScriptableEntity *instance = nullptr;
        ScriptableEntity *(*instantiate_fn)();
        void (*destroy_fn)(Behavior *);

        template <typename T>
        void bind()
        {
            instantiate_fn = [](){ return static_cast<ScriptableEntity *>(new T()); };
            destroy_fn = [](Behavior *behavior){delete behavior->instance;behavior->instance = nullptr; };
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

    void SerializeTransform( nlohmann::json &data, std::shared_ptr<Scene> registry, entt::entity entity);
    void SerializeCamera(nlohmann::json &data, entt::registry &registry, entt::entity entity);
    void SerializeMeshAsset(nlohmann::json &data, entt::registry &registry, entt::entity entity);
    void SerializeCameraEditor(nlohmann::json &data, entt::registry &registry, entt::entity entity);
    void DeserializeTransform(nlohmann::json &data, TransformComponent &);
    void DeserializeCamera(nlohmann::json &data, CameraComponent &);
    void DeserializeMeshAsset(nlohmann::json &data, ModelAssetComponent &);
    class CameraEditor;
    void DeserializeCameraEditor(nlohmann::json &data, CameraEditor &);

    //Component Referencies
    void ResolveReferencesInTransform(TransformComponent& transform_component, std::shared_ptr<Scene> scene);

/*
    void assign_reference(std::shared_ptr<Scene> scene,Reference reference);
    void resolve_component_references(std::shared_ptr<Scene> scene,std::vector<Reference>& references);*/

}

#endif