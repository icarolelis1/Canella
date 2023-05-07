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
    void load_meshes_from_scene(const std::string& assetsFolder,const std::weak_ptr<Scene> scene);
    void load_mesh_from_disk(const std::string& assetsFolder,MeshAssetComponent& mesh_asset_component);
    CameraComponent& get_main_camera(const std::weak_ptr<Scene> scene);
    void get_meshes_on_scene(Drawables& drawables,const std::weak_ptr<Scene> scene);
    
    class GlfwWindow;
    void update_camera(CameraComponent& camera_component,GlfwWindow& window);
}
#endif
