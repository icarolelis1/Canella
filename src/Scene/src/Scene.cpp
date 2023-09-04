#include "Entity.h"
#include "Scene/Scene.h"
#include "Systems/Systems.h"
#include "Render/Render.h"
#include "AssetSystem/AssetSystem.h"
#include "CanellaUtility/CanellaUtility.h"

std::uniform_int_distribution<uint64_t> uniform_distribution;
Canella::Entity& Canella::Scene::CreateEntity()
{
    entt::entity entt_entity = registry.create();
    auto shared_ptr = this->shared_from_this();
    auto entity =  std::make_shared<Entity>(entt_entity, shared_ptr);
    entity->uuid =  uniform_distribution(random_engine);
    entity->name = "Entity"  + std::to_string(static_cast<uint32_t>(entt_entity));
    entity->add_component<TransformComponent>();
    entityLibrary[entt_entity] = std::move(entity);
    return *entityLibrary[entt_entity];
}

void Canella::Scene::init_systems()
{
    //Loads all the scenes in the scene before run time
   // load_meshes_from_scene( asset_folder, this, render );

    //Get all the meshes loaded from the scene
    std::vector<ModelMesh> meshes;
    get_static_meshes_on_scene(meshes,this);
    //For static meshes the transform needs to be uploaded before enqueing the drawables
    update_transforms(this);
    //Send the meshes to be rendered by the renderer
    render->enqueue_drawables(meshes);
    //Create render graph
    render->create_render_graph_resources();
    //Gets the reference for the main Camera
    main_camera = get_main_camera(this);
    //starts the scripts calling on_start
    start_scripts(this);

}

Canella::Scene::Scene(const std::string &_asset_folder, Canella::Render *_render) :
        asset_folder(_asset_folder),
        render(_render) {
}

void Canella::Scene::update_systems(float frame_time) {
    update_scripts(this,frame_time);
    //Todo calculate the model matrix in shader side not CPU.
    //Update scene transforms
    update_transforms(this);
}

Canella::Entity &Canella::Scene::CreateEntity( uint64_t _uuid ) {
    entt::entity entt_entity = registry.create();
    auto shared_ptr = this->shared_from_this();
    auto entity =  std::make_shared<Entity>(entt_entity, shared_ptr);
    entity->uuid =  _uuid;
    entity->name = "Entity"  + std::to_string(static_cast<uint32_t>(entt_entity));
    entity->add_component<TransformComponent>();
    entityLibrary[entt_entity] = std::move(entity);
    return *entityLibrary[entt_entity];
}

Canella::Entity& Canella::Scene::get_entity_by_uuid( uint64_t uuid ) {
    for(auto& [entt,entity] : entityLibrary)
        if(entity->uuid == uuid) return *entity.get();
}


