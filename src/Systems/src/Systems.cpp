#include "Systems/Systems.h"
#include "Window/Window.h"

#include "Render/Render.h"
using namespace Canella;

void Canella::load_meshes_from_scene(const std::string& assets_folder,const std::weak_ptr<Scene> scene)
{
    const auto scene_ref = scene.lock();
    if (!scene_ref)return;
    for (auto& [entt_value,entity] : scene_ref->m_EntityLibrary)
        if (entity->has_component<ModelAssetComponent>())
        {
            auto& mesh_asset = entity->get_component<ModelAssetComponent>();
            //mesh_asset.mesh.model_matrix = &entity->get_component<TransformComponent>().modelMatrix;
            load_mesh_from_disk(assets_folder, mesh_asset);
        }
}

void Canella::load_mesh_from_disk(const std::string& assets_folder, ModelAssetComponent& mesh_asset_component)
{
    load_asset_mesh(mesh_asset_component.mesh, assets_folder, mesh_asset_component.source);
}

/**
 * \brief Returns the main camera
 * \param scene scene object
 * \return returns the main Camera
 */
CameraComponent* Canella::get_main_camera(const std::weak_ptr<Scene> scene)
{
    const auto scene_ref = scene.lock();
    assert(scene_ref);

    for (auto& [entt,entity] : scene_ref->m_EntityLibrary)
        if (entity->has_component<CameraComponent>())
            return &entity->get_component<CameraComponent>();
   return nullptr;
}

void Canella::get_meshes_on_scene(Drawables& drawables,const std::weak_ptr<Scene> scene)
{
    const auto scene_ref = scene.lock();
    assert(scene_ref);
    for (auto [key,value]: scene_ref->m_EntityLibrary)
        if (value->has_component<ModelAssetComponent>()) {
            const auto& [mesh, source, isStatic] = value->get_component<ModelAssetComponent>();
            drawables.push_back(mesh);
        }
}

void Canella::update_transforms(const std::weak_ptr<Scene> scene)
{
    const auto scene_ref = scene.lock();
    assert(scene_ref);
    for (auto [key,value]: scene_ref->m_EntityLibrary)
        if (value->has_component<TransformComponent>()) {
            auto& transform= value->get_component<TransformComponent>();
            transform.modelMatrix = glm::mat4(1.0f);
            //glm::quat rot = glm::angleAxis(glm::radians(0.f),glm::vec3(0,1,0));
            //transform.modelMatrix *= toMat4(rot);
            transform.modelMatrix = glm::translate(transform.modelMatrix,transform.position);
        }
}

void Canella::update_scripts(std::weak_ptr<Scene> scene,float frame_time)
{
    const auto scene_ref = scene.lock();
    assert(scene_ref);
    scene_ref->m_registry.view<Behavior>().each([=](auto entity, auto& behavior)
    {
        if(!behavior.instance)
        {
            behavior.instance = behavior.instantiate_fn();
            behavior.instance->entt_entity = entity;
        }
    behavior.instance->on_update(frame_time);
    });
}

void Canella::start_scripts(std::weak_ptr<Scene> scene)
{
    const auto scene_ref = scene.lock();
    assert(scene_ref);
    scene_ref->m_registry.view<Behavior>().each([=](auto entity, auto& behavior)
    {
        if(!behavior.instance)
        {
            behavior.instance = behavior.instantiate_fn();
            behavior.instance->entt_entity = entity;
        }
        behavior.instance->on_start();
    });
}
