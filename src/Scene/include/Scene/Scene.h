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
    struct CameraComponent;
    struct MaterialDescription;
    using EntityLibrary = std::unordered_map<entt::entity, std::shared_ptr<Entity>>;
    using MaterialLibrary = std::unordered_map<std::string, MaterialDescription>;
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
        Entity& CreateEntity();

        /**
        * @brief create an entity to the scene
        * @return
        */
        Entity& CreateEntity(uint64_t uuid);

        void init_systems();
        /**
         * @brief update the component system
         * @param delta time between last frame and current frame
         */
        void update_systems(float delta);

        Entity& get_entity_by_uuid(uint64_t uuid);

        /**
         * @brief Creates an entity parented by the root node
         * @return
         */
        Entity& create_root_parented_entity();

        std::unique_ptr<Canella::Entity> root;
        CameraComponent * main_camera;
        entt::registry registry;
        EntityLibrary  entityLibrary;
        MaterialLibrary material_library;
    private:

        bool init_root();

        std::vector<uint32_t> static_geometry;
        Canella::Render* render;
        std::string asset_folder;
    };
}

#endif