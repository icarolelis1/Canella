#include "Systems/Systems.h"
#include "Window/Window.h"
#include "Mesh/Mesh.h"
#include "Render/Render.h"
using namespace Canella;

void Canella::load_meshes_from_scene(const std::string &assets_folder, Scene *const scene)
{
    for (auto &[entt_value, entity] : scene->m_EntityLibrary)
        if (entity->has_component<ModelAssetComponent>())
        {
            auto &mesh_asset = entity->get_component<ModelAssetComponent>();
            // mesh_asset.mesh.model_matrix = &entity->get_component<TransformComponent>().modelMatrix;
            load_mesh_from_disk(assets_folder, mesh_asset);
        }
}

void Canella::load_mesh_from_disk(const std::string &assets_folder, ModelAssetComponent &mesh_asset_component)
{
    Canella::MeshProcessing::load_asset_mesh(mesh_asset_component.mesh, assets_folder, mesh_asset_component.source);
}

/**
 * \brief Returns the main camera
 * \param scene scene object
 * \return returns the main Camera
 */
CameraComponent *Canella::get_main_camera(Scene *const scene)
{
    for (auto &[entt, entity] : scene->m_EntityLibrary)
        if (entity->has_component<CameraComponent>())
            return &entity->get_component<CameraComponent>();
    return nullptr;
}

void Canella::get_static_meshes_on_scene(Drawables &drawables, Scene *const scene)
{
    for (auto [key, value] : scene->m_EntityLibrary)
        if (value->has_component<ModelAssetComponent>())
        {
            const auto &[mesh, source, isStatic] = value->get_component<ModelAssetComponent>();
            if (isStatic)
                drawables.push_back(mesh);
        }
}

void Canella::update_transforms(Scene *const scene)
{
    for (auto [key, value] : scene->m_EntityLibrary)
        if (value->has_component<TransformComponent>())
        {
            auto &transform = value->get_component<TransformComponent>();
            transform.modelMatrix = glm::mat4(1.0f);
            glm::quat q1 = glm::angleAxis(glm::radians(transform.rotation.x), glm::vec3(1, 0, 0));
            glm::quat q2 = glm::angleAxis(glm::radians(transform.rotation.y), glm::vec3(0, 1, 0));
            glm::quat q3 = glm::angleAxis(glm::radians(transform.rotation.z), glm::vec3(0, 0, 1));
            glm::quat orientation = q1 * q2 * q3;
            transform.modelMatrix = glm::translate(transform.modelMatrix, transform.position) * glm::mat4_cast(orientation);
            transform.modelMatrix = glm::scale(transform.modelMatrix, transform.scale);
            transform.modelMatrix;
            if (transform.parent != nullptr)
                transform.modelMatrix = transform.parent->modelMatrix * transform.modelMatrix;

            // glm::quat rot = glm::angleAxis(glm::radians(0.f),glm::vec3(0,1,0));
            // transform.modelMatrix *= toMat4(rot);
        }
}

void Canella::update_scripts(Scene *scene, float frame_time)
{
    scene->m_registry.view<Behavior>().each([=](auto entity, auto &behavior)
                                            {
        if(!behavior.instance)
        {
            behavior.instance = behavior.instantiate_fn();
            behavior.instance->entt_entity = entity;
        }
    behavior.instance->on_update(frame_time); });
}

void Canella::start_scripts(Scene *scene)
{
    scene->m_registry.view<Behavior>().each([=](auto entity, auto &behavior)
                                            {
        if(!behavior.instance)
        {
            behavior.instance = behavior.instantiate_fn();
            behavior.instance->entt_entity = entity;
        }
        behavior.instance->on_start(); });
}
