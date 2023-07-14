#include "AssetSystem/AssetSystem.h"
#include "Render/Render.h"
#include "Mesh/Mesh.h"

Canella::AssetSystem &Canella::AssetSystem::instance()
{
    static AssetSystem asset_system;
    return asset_system;
}

void Canella::AssetSystem::set_project_src(const char *src)
{
    project_src = src;
}

void Canella::AssetSystem::load_asset(ModelAssetComponent &asset)
{
    // Load mesh data and computes the meshlets
    Canella::MeshProcessing::load_asset_mesh(asset.mesh, project_src, asset.source);
    OnLoadedModel.invoke(asset.mesh);
}