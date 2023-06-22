#pragma once
#ifndef ASSET_SYSTEM
#define ASSET_SYSTEm
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
        void load_asset(ModelAssetComponent &asset);

        // Events
        Event<ModelMesh &> OnLoadedModel;

    private:
        AssetSystem() = default;

        const char *project_src;
    };
}

#endif