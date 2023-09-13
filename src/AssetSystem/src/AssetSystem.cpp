#include <filesystem>
#include "AssetSystem/AssetSystem.h"
#include "Mesh/Mesh.h"
#include "JobSystem/JobSystem.h"
#include "Render/Framework.h"


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
    }

private :
    Canella::ModelAssetComponent& asset;
    Canella::AssetSystem* asset_system;
    Canella::Render* render;

    void onCompleteCallback() override
    {
        render->enqueue_drawable(asset.mesh);
    }
};


struct LoadTextureJob : Canella::JobSystem::JobDetail
{
    LoadTextureJob( std::string _texture_path,Canella::Render* _renderer) : texture_path(_texture_path),renderer(_renderer)
    {}

    void execute() override
    {
        Canella::create_texture(renderer,texture_path);
    }
    ~LoadTextureJob() = default;

private :
    int code;
    std::string texture_path;
    Canella::Render* renderer;
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
    JobSystem::schedule(std::move(job));
}

std::string Canella::AssetSystem::get_project_src() {
      auto str = std::string(project_src);
      return str;
}

void Canella::AssetSystem::set_renderer( Canella::Render *_render ) {
    renderer = _render;
}

void Canella::AssetSystem::load_asset( Canella::ModelAssetComponent &asset ) {
    auto project_src = get_project_src();
    Canella::MeshProcessing::load_asset_mesh(asset.mesh, project_src, asset.source);
    Canella::MeshProcessing::load_instance_data(asset.mesh, project_src, asset.instance_src);

}

void Canella::AssetSystem::load_material_async( Canella::MaterialDescription material) {
    for(auto& slot : material.texture_slots)
    {
        const auto project_path = std::filesystem::absolute(project_src);
        JobSystem::CanellaJob job((std::make_shared<LoadTextureJob>(project_path.string() +"\\" + "Textures" + "\\" + slot.texture_source,renderer)));
        JobSystem::schedule(std::move(job));
    }
}
