#include "Systems/Systems.h"
#include "AssetSystem/AssetSystem.h"
#include "JobSystem/JobSystem.h"
#include "Render/Framework.h"
using namespace Canella;

void Canella::load_initial_meshes_on_scene( Drawables &drawables, Scene *const scene)
{
    auto& asset_system = Canella::AssetSystem::instance();
    for(auto [entity, mesh_asset] : scene->registry.view<ModelAssetComponent>().each())
    {
        asset_system.load_asset( mesh_asset);
        drawables.push_back(mesh_asset.mesh);
    }
}

void Canella::load_initial_materials_on_scene( Scene *const scene,Render* render, MaterialCollection& materials ) {
    auto& asset_system = Canella::AssetSystem::instance();
    for(auto& material_description : scene->materials_used_in_scene)
        asset_system.load_material_async( material_description,materials);
    //wait until all initial materials are loaded
    JobSystem::wait();
    for(auto& material : materials.collection)
        Canella::allocate_material_data(render,material);

}

CameraComponent *Canella::get_main_camera(Scene *const scene)
{
    for (auto &[entt, entity] : scene->entityLibrary)
        if (entity->has_component<CameraComponent>())
            return &entity->get_component<CameraComponent>();
    return nullptr;
}

void Canella::update_transforms(Scene * const scene)
{
    for (auto [key, value] : scene->entityLibrary)
    {
        auto &transform = scene->registry.view<TransformComponent>().get<TransformComponent>(key);
        transform.model_matrix = glm::mat4( 1.0f);
        auto trans_matrix   =  glm::translate( glm::mat4(1.0f), transform.position);
        auto rot_matrix     =  glm::mat4_cast( transform.orientation );
        transform.model_matrix = trans_matrix*rot_matrix;
        transform.model_matrix   =  glm::scale( transform.model_matrix, transform.scale);
        transform.rotation = glm::eulerAngles(transform.orientation) * 57.2958f ;
        if (transform.parent != nullptr)
            transform.model_matrix = transform.parent->model_matrix * transform.model_matrix;
    }

    //todo figure why this doesnt work
/*    scene->registry.view<TransformComponent>()
            .each([](const auto& entity,auto& transform)
                 {
                     transform.model_matrix = glm::mat4( 1.0f);
                     auto trans_matrix   =  glm::translate( glm::mat4(1.0f), transform.position);
                     auto rot_matrix     =  glm::mat4_cast( transform.orientation );
                     transform.model_matrix = trans_matrix * rot_matrix;
                     transform.model_matrix   =  glm::scale( transform.model_matrix, transform.scale);
                     transform.rotation = glm::eulerAngles(transform.orientation) * 57.2958f ;

                     if (transform.parent != nullptr)
                         transform.model_matrix = transform.parent->model_matrix * transform.model_matrix;
                 });*/

}

void Canella::update_scripts(Scene *scene, float frame_time)
{
    scene->registry.view<Behavior>()
            .each( [=]( auto entity, auto &behavior)
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
    scene->registry.view<Behavior>()
            .each( [=]( auto entity, auto &behavior)
                   {
                       if(!behavior.instance)
                       {
                           behavior.instance = behavior.instantiate_fn();
                           behavior.instance->entt_entity = entity;
                       }
                       behavior.instance->on_start();
                   });
}

