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
    CameraComponent* get_main_camera(Scene *const scene);
    /**
 * \brief get a reference for all the Meshes in the scene
 * \param drawables Reference to vector containing the meshes
 * \param scene The Scene
 */
    void load_initial_meshes_on_scene( Drawables& drawables, Scene *const scene);

    /**
     * @brief Load the textures associated with materials at start
     * @param scene The Scene
     */
    void load_initial_materials_on_scene( Scene *const scene );

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
}
#endif
