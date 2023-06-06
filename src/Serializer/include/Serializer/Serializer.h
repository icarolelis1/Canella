#pragma once
#ifndef SERIALIZER
#define SERIALIZER
#include <memory>
#include <string>
#include "Entity.h"

namespace Canella
{
    class Scene;
    class Serializer
    {
    public:
        Serializer() = default;
        explicit Serializer(const std::string& projectFolder);
        Serializer(const Serializer &other) = delete;
        ~Serializer() = default;
        /**
        * \brief Serialize the scene to a file
        * \param scene
        */
        void Serialize(std::weak_ptr<Scene> scene,const std::string& projectPath);
        /**
         * @brief Deserialize the scene
         * @param scene scene to be deserialized
         */
        void Deserialize(std::weak_ptr<Scene> scene);
        
    private:
        void LoadEntities(std::shared_ptr<Scene> scene, const std::string& filepath);
        /**
        * \brief Builds the components for each entity in the config File
        * \param entity entity the component is associated to
        * \param components_data Data Serialized for each component
        */
        void LoadComponents(std::shared_ptr<Scene> scene,
            const entt::entity entity,nlohmann::json &components_data);

        /**
         * @brief writes the all the entitites data into a json file
         * @param scene the scene to deserialize
         * @param file_path source file to write the output
         */
        void DeserializeEntities(const std::shared_ptr<Canella::Scene> scene, const std::string& file_path);

        std::string m_ProjectFolder;
    };
}

#endif