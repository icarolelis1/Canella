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
        void Serialize(std::weak_ptr<Scene> scene,const std::string& projectPath);
        void Deserialize(std::weak_ptr<Scene> scene);
        
    private:
        void LoadEntities(std::shared_ptr<Scene> scene, const std::string& filepath);
        void LoadComponents(std::shared_ptr<Scene> scene,
            const entt::entity entity,nlohmann::json &components_data);
        void DeserializeEntities(const std::shared_ptr<Canella::Scene> scene, const std::string& basic_string);

        std::string m_ProjectFolder;
    };
}

#endif