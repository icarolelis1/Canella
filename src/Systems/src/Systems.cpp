#include "Systems/Systems.h"
#include "Window/Window.h"
using namespace Canella;

void Canella::load_meshes_from_scene(const std::string& assets_folder,const std::weak_ptr<Scene> scene)
{
    const auto scene_ref = scene.lock();
    if (!scene_ref)return;
    for (auto& [entt_value,entity] : scene_ref->m_EntityLibrary)
        if (entity.HasComponent<MeshAssetComponent>())
        {
            auto& mesh_asset = entity.GetComponent<MeshAssetComponent>();
            mesh_asset.mesh.modelMatrix = &entity.GetComponent<TransformComponent>().modelMatrix;
            load_mesh_from_disk(assets_folder, mesh_asset);
        }
}

void Canella::load_mesh_from_disk(const std::string& assets_folder, MeshAssetComponent& mesh_asset_component)
{
    LoadAssetMesh(mesh_asset_component.mesh, assets_folder, mesh_asset_component.source);
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
        if (entity.HasComponent<CameraComponent>())
            return entity.GetComponent<CameraComponent>();
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
        if (value.HasComponent<MeshAssetComponent>()) {
            const auto& [mesh, source, isStatic] = value.GetComponent<MeshAssetComponent>();
            drawables.push_back(mesh);
        }
}
void Canella::update_camera(CameraComponent& camera_component, GlfwWindow& window)
{
    const auto [width, height] = window.getExtent();
    const auto aspect = static_cast<float>(width / height);
    const auto perspective = glm::perspective(camera_component.fovy, aspect, camera_component.zNear,
                                              camera_component.zFar);

    const auto view = lookAt(camera_component.position,
        camera_component.euler.front, camera_component.euler.up);
    camera_component.viewProjection = view * perspective;
}
