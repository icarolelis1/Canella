#include "AssetSystem/AssetSystem.h"
#include "Mesh/Mesh.h"
#include "JobSystem/JobSystem.h"

struct LoadModelJob : Canella::JobSystem::JobDetail
{
    LoadModelJob(Canella::ModelAssetComponent &_asset,
                 Canella::AssetSystem *_asset_system,
                 Canella::Render* _render) :
                 asset(_asset),asset_system(_asset_system),render(_render)
    {}

    void execute() override
    {
        auto project_src = asset_system->get_project_src();
        Canella::MeshProcessing::load_asset_mesh(asset.mesh, project_src, asset.source);
        //Load all textures
    }
    bool immediatly_dispatch = false;
private :
    Canella::ModelAssetComponent& asset;
    Canella::AssetSystem* asset_system;
    Canella::Render* render;

    void onCompleteCallback() override
    {
        render->enqueue_drawable(asset.mesh);
    }
};

Canella::AssetSystem &Canella::AssetSystem::instance()
{
    static AssetSystem asset_system;
    return asset_system;
}

void Canella::AssetSystem::set_project_src(const char *src)
{
    project_src = src;
}

void Canella::AssetSystem::async_load_asset(ModelAssetComponent &asset)
{
    auto load_job = std::make_shared<LoadModelJob>(asset,this,renderer);
    JobSystem::CanellaJob job(load_job);
    JobSystem::schedule(job);
}

std::string Canella::AssetSystem::get_project_src() {
      auto str = std::string(project_src);
      return str;
}

void Canella::AssetSystem::set_renderer( Canella::Render *_render ) {
    renderer = _render;
}
