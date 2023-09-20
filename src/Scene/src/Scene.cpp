#include "Entity.h"
#include "Scene/Scene.h"
#include "Systems/Systems.h"
#include "Render/Render.h"
#include "CanellaUtility/CanellaUtility.h"

std::uniform_int_distribution<uint64_t> uniform_distribution;

Canella::Entity& Canella::Scene::CreateEntity()
{
    if(root == nullptr) init_root();

    entt::entity entt_entity = registry.create();
    auto shared_ptr = this->shared_from_this();
    auto entity =  std::make_shared<Entity>(entt_entity, shared_ptr);
    entity->uuid =  uniform_distribution(random_engine);
    entity->name = "Entity"  + std::to_string(static_cast<uint32_t>(entt_entity));
    auto& transform = entity->add_component<TransformComponent>();
    transform.owner = entity.get();
    entityLibrary[entt_entity] = std::move(entity);
    return *entityLibrary[entt_entity];
}

void Canella::Scene::init_scene()
{
    update_transforms(this);
    //Create render graph
    //Gets the reference for the main Camera
    main_camera = get_main_camera(this);
    //starts the scripts calling on_start
    start_scripts(this);

}

void Canella::Scene::update_systems(float frame_time) {
    update_scripts(this,frame_time);
    update_transforms(this);
}

Canella::Entity &Canella::Scene::CreateEntity( uint64_t _uuid ) {

    if(root == nullptr)
        init_root();

    entt::entity entt_entity = registry.create();
    auto shared_ptr = this->shared_from_this();
    auto entity =  std::make_shared<Entity>(entt_entity, shared_ptr);
    entity->uuid =  _uuid;
    entity->name = "Entity"  + std::to_string(static_cast<uint32_t>(entt_entity));
    auto& transform = entity->add_component<TransformComponent>();
    transform.owner = entity.get();
    entityLibrary[entt_entity] = std::move(entity);
    return *entityLibrary[entt_entity];
}

Canella::Entity& Canella::Scene::get_entity_by_uuid( uint64_t uuid ) {
    for(auto& [entt,entity] : entityLibrary)
        if(entity->uuid == uuid) return *entity.get();
}

bool Canella::Scene::init_root() {

    entt::entity entt_entity = registry.create();
    auto shared_ptr = this->shared_from_this();
    auto entity =  std::make_shared<Entity>(entt_entity, shared_ptr);
    entity->uuid =  0;
    entity->name = "Scene_Root";
    auto& root_transform = entity->add_component<TransformComponent>();
    root_transform.owner = entity.get();
    entityLibrary[entt_entity] = std::move(entity);
    root = std::make_unique<Entity>(*entityLibrary[entt_entity]) ;
    return true;
}

Canella::Entity &Canella::Scene::create_root_parented_entity() {
    entt::entity entt_entity = registry.create();
    auto shared_ptr = this->shared_from_this();
    auto entity =  std::make_shared<Entity>(entt_entity, shared_ptr);
    entity->uuid =  uniform_distribution(random_engine);
    entity->name = "Entity"  + std::to_string(static_cast<uint32_t>(entt_entity));
    auto& transform = entity->add_component<TransformComponent>();
    transform.owner = entity.get();
    entityLibrary[entt_entity] = std::move(entity);

    auto& root_transform = root->get_component<TransformComponent>();
    root_transform.children.push_back(&transform);
    transform.parent = &root_transform;
    return *entityLibrary[entt_entity];

}


