#pragma once
#ifndef ASSET_SYSTEM
#define ASSET_SYSTEM
#include "VulkanRender/VulkanRender.h"
#include <Eventsystem/Eventsystem.hpp>
#include "Components/Components.h"

namespace Canella
{
    using namespace RenderSystem::VulkanBackend;

    class AssetSystem
    {
    public:
        static AssetSystem &instance();

        /**
         * @brief set the assets source directory
         * @param src source directory that contains the meshes
         */
        void set_project_src(const char *src);

        /**
         * @brief loads a mesh object
         * @param asset mesh asset to be loaded
         */
        void async_load_asset(ModelAssetComponent &asset);

        /**
         * @brief Get the source directory of the project
         * @return the source directory of the project
         */
        std::string get_project_src();

        /**
         * @brief Injects an abstract renderer into asset system
         * @param render application renderer
         */
        void set_renderer(Canella::Render* render);

    private:
        Canella::Render* renderer;
        AssetSystem() = default;
        const char *project_src;
    };
}

#endif