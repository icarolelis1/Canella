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
    void load_meshes_from_scene( const std::string &assetsFolder, Scene *const scene, Render *p_render );

    CameraComponent* get_main_camera(Scene *const scene);
    /**
 * \brief get a reference for all the Meshes in the scene
 * \param drawables Reference to vector containing the meshes
 * \param scene The Scene
 */
    void get_static_meshes_on_scene(Drawables& drawables, Scene *const scene);
    /**
     * @brief update all the transforms calculating the model matrix
     * @param scene the scene containing the transforms
     */
    void update_transforms(Scene *const scene);

    /**
     * @brief update the scene scripts calling on_update
     * @param scene
     */
    void update_scripts(Scene *scene, float frame_time);

    /**
     * Starts the scene scripts calling on_start method
     * @param scene
     */
    void start_scripts(Scene *scene);
    class GlfwWindow;
    void update_camera(CameraComponent& camera_component,GlfwWindow& window);
}
#endif
