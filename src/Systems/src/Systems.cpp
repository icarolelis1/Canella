#include "Systems/Systems.h"
#include "AssetSystem/AssetSystem.h"
#include "JobSystem/JobSystem.h"

using namespace Canella;

Entity& get_entity_by_uid(Scene *const scene,uint64_t uid)
{
    for (auto [key, value] : scene->entityLibrary) {
        if(value->uuid == uid)
        {
            return *value.get();
        }
    }
}

/*void Canella::resolve_component_referencies(Scene *const scene)
{
    for (auto [key, value] : scene->entityLibrary) {
        if(!value->is_dirty)continue;
        auto &transform = value->get_component<TransformComponent>();
        if(transform.reference_to_transform.uid != 0)
        {
           auto& parent_transform = get_entity_by_uid(scene,transform.reference_to_transform.uid).get_component<TransformComponent>();
           transform.parent = &parent_transform;
        }
    }
}*/


void Canella::load_meshes_from_scene( const std::string &assets_folder, Scene *const scene, Canella::Render *p_render )
{
    auto& asset_system = Canella::AssetSystem::instance();
    std::list<std::string> material_names;
    for (auto &[entt_value, entity] : scene->entityLibrary)
        if (entity->has_component<ModelAssetComponent>())
        {
            auto &mesh_asset = entity->get_component<ModelAssetComponent>();
            material_names.push_back(mesh_asset.material_name);
            // mesh_asset.mesh.model_matrix = &entity->get_component<TransformComponent>().model_matrix;
            asset_system.async_load_asset( mesh_asset);
        }
    Canella::JobSystem::wait();
}


void Canella::load_initial_meshes_on_scene( Drawables &drawables, Scene *const scene)
{
    auto& asset_system = Canella::AssetSystem::instance();
    for (auto [key, entity] : scene->entityLibrary)
        if (entity->has_component<ModelAssetComponent>())
        {
            auto &mesh_asset = entity->get_component<ModelAssetComponent>();
            //Get all the materials in use at start
            asset_system.load_asset( mesh_asset);
            drawables.push_back(mesh_asset.mesh);
            Logger::Debug("Size of mesh %d",sizeof(drawables.end()->positions));
        }
}

void Canella::load_initial_materials_on_scene( Scene *const scene) {
    auto& asset_system = Canella::AssetSystem::instance();
    for (auto [key, entity] : scene->entityLibrary)
        if (entity->has_component<ModelAssetComponent>())
        {
            auto &mesh_asset = entity->get_component<ModelAssetComponent>();
            asset_system.load_material_async( scene->material_library[mesh_asset.material_name] );
        }
    JobSystem::wait();

}

CameraComponent *Canella::get_main_camera(Scene *const scene)
{
    for (auto &[entt, entity] : scene->entityLibrary)
        if (entity->has_component<CameraComponent>())
            return &entity->get_component<CameraComponent>();
    return nullptr;
}

void resolve_hierarchy_transforms(Scene *const scene)
{
    for (auto [key, value] : scene->entityLibrary)
        if (value->has_component<TransformComponent>())
        {
            auto &transform = value->get_component<TransformComponent>();
        }
}

void Canella::update_transforms(Scene *const scene)
{
    for (auto [key, value] : scene->entityLibrary)
        if (value->has_component<TransformComponent>())
        {
            auto &transform = value->get_component<TransformComponent>();
            transform.model_matrix = glm::mat4( 1.0f);
            auto trans_matrix   =  glm::translate( glm::mat4(1.0f), transform.position);
            auto rot_matrix     =  glm::mat4_cast( transform.orientation );
            transform.model_matrix = trans_matrix*rot_matrix;
            transform.model_matrix   =  glm::scale( transform.model_matrix, transform.scale);
            transform.rotation = glm::eulerAngles(transform.orientation) * 57.2958f ;

            if (transform.parent != nullptr)
                transform.model_matrix = transform.parent->model_matrix * transform.model_matrix;
        }
}

void Canella::update_scripts(Scene *scene, float frame_time)
{
    scene->registry.view<Behavior>().each( [=]( auto entity, auto &behavior)
                                           {
                                               if(!behavior.instance)
                                               {
                                                   behavior.instance = behavior.instantiate_fn();
                                                   behavior.instance->entt_entity = entity;
                                               }
                                               behavior.instance->on_update(frame_time);
                                           });
}

void Canella::start_scripts(Scene *scene)
{
    scene->registry.view<Behavior>().each( [=]( auto entity, auto &behavior)
                                            {
        if(!behavior.instance)
        {
            behavior.instance = behavior.instantiate_fn();
            behavior.instance->entt_entity = entity;
        }
        behavior.instance->on_start(); });
}

