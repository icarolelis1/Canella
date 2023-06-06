#pragma once
#ifndef CANELLA_SYSTEMS
#define CANELLA_SYSTEMS
#include <Entity.h>
#include "Components/Components.h"
#include "glm/mat4x4.hpp"
#include "Scene/Scene.h"
#include <memory>
namespace Canella
{

    /**
     * @brief load all the mesh assets from the scene
     * @param assets_folder subfolder inside project that contains the assets
     * @param scene the scene containing the meshes
     */
    void load_meshes_from_scene(const std::string& assetsFolder,const std::weak_ptr<Scene> scene);
    /**
     * @brief load a mesh in assetsFolder path
     * @param assetsFolder
     * @param mesh_asset_component
     */
    void load_mesh_from_disk(const std::string& assetsFolder,ModelAssetComponent& mesh_asset_component);
    CameraComponent* get_main_camera(const std::weak_ptr<Scene> scene);
    /**
 * \brief get a reference for all the Meshes in the scene
 * \param drawables Reference to vector containing the meshes
 * \param scene The Scene
 */
    void get_meshes_on_scene(Drawables& drawables,const std::weak_ptr<Scene> scene);
    /**
     * @brief update all the transforms calculating the model matrix
     * @param scene the scene containing the transforms
     */
    void update_transforms(const std::weak_ptr<Scene> scene);

    /**
     * @brief update the scene scripts calling on_update
     * @param scene
     */
    void update_scripts(std::weak_ptr<Scene> scene,float frame_time);

    /**
     * Starts the scene scripts calling on_start method
     * @param scene
     */
    void start_scripts(std::weak_ptr<Scene> scene);
    class GlfwWindow;
    void update_camera(CameraComponent& camera_component,GlfwWindow& window);
}
#endif
