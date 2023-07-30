#pragma once
#ifndef SCENE
#define SCENE
#include "entt.hpp"
#include "Logger/Logger.hpp"
#include "json.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace Canella {
    class Render;
    class Entity;
    class CameraComponent;
    using EntityLibrary = std::unordered_map<entt::entity, std::shared_ptr<Entity>>;
    class Scene : public std::enable_shared_from_this<Scene> {
    public:
        /**
         * @brief Creates the scene
         * @param asset_folder Relative directory the scene will look for meshes
         * @param render application renderer
         */
        Scene(const std::string& asset_folder,Canella::Render* render);
        ~Scene() = default;
        /**
         * @brief create an entity to the scene
         * @return
         */
        Entity CreateEntity();
        void init_systems();
        /**
         * @brief update the component system
         * @param delta time between last frame and current frame
         */
        void update_systems(float delta);
        CameraComponent * main_camera;
        entt::registry registry;
        EntityLibrary  entityLibrary;
    private:
        std::vector<uint32_t> static_geometry;
        Canella::Render* render;
        std::string asset_folder;
    };
}

#endif