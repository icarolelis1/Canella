#include "Systems/Systems.h"
#include "Window/Window.h"
using namespace Canella;

void Canella::load_meshes_from_scene(const std::string& assets_folder,const std::weak_ptr<Scene> scene)
{
    const auto scene_ref = scene.lock();
    if (!scene_ref)return;
    for (auto& [entt_value,entity] : scene_ref->m_EntityLibrary)
        if (entity->has_component<ModelAssetComponent>())
        {
            auto& mesh_asset = entity->get_component<ModelAssetComponent>();
            mesh_asset.mesh.model_matrix = &entity->get_component<TransformComponent>().modelMatrix;
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
CameraComponent& Canella::get_main_camera(const std::weak_ptr<Scene> scene)
{
    const auto scene_ref = scene.lock();
    assert(scene_ref);

    for (auto& [entt,entity] : scene_ref->m_EntityLibrary)
        if (entity->has_component<CameraComponent>())
            return entity->get_component<CameraComponent>();
    const auto camera_component = new CameraComponent();
    return *camera_component;
}

/**
 * \brief get a reference for all the Meshes in the scene
 * \param drawables Reference to vector containing the meshes
 * \param scene The Scene
 */
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
void Canella::update_camera(CameraComponent& camera_component, GlfwWindow& window)
{
    const auto [width, height] = window.getExtent();
    if(height == 0 || width == 0) return;
    const auto aspect = static_cast<float>(width / height);
    const auto perspective = glm::perspective(camera_component.fovy, aspect, camera_component.zNear,
                                              camera_component.zFar);

    const auto view = lookAt(camera_component.position,
        camera_component.euler.front, camera_component.euler.up);
    camera_component.viewProjection = view * perspective;
}
