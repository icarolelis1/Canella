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
    LoadTextureJob( std::string _texture_path,Canella::Render* _renderer) :
    texture_path(_texture_path),
    renderer(_renderer)
    {}

    void execute() override
    {
        texture_accessor = Canella::create_texture(renderer,texture_path);
    }
    ~LoadTextureJob() = default;

    std::string texture_path;
    Canella::Render* renderer;
    uint64_t texture_accessor;
};


struct LoadMaterialFromDisk : Canella::JobSystem::JobDetail
{
    LoadMaterialFromDisk( std::string _project_src,Canella::Render* _renderer,
                          Canella::MaterialDescription _material_desc,
                          Canella::MaterialCollection* _materials ) :
            project_src(_project_src),
            renderer(_renderer),
            material_desc(_material_desc),
            materials(_materials)

    {}

    void execute() override
    {
        //Load textures
        new_material.name = material_desc.name;
        for(auto&slot : material_desc.texture_slots)
        {
            const auto project_path = std::filesystem::absolute(project_src);
            auto texture_accessor = Canella::create_texture(renderer,project_path.string()+"\\" + "Textures" + "\\" + slot.texture_source);
            Canella::Logger::Info("Loaded texture %s",slot.texture_source.c_str());
            new_material.texture_accessors.push_back(texture_accessor);
        }
    }

    void onCompleteCallback() override
    {
        materials->material_loaded_record[new_material.name] = true;
        Canella::Logger::Info("Finished Loading Material texture %s",new_material.name.c_str());
        materials->collection.push_back(std::move(new_material));
    }

    ~LoadMaterialFromDisk() = default;
    std::string project_src;
    Canella::Render* renderer;
    Canella::MaterialCollection* materials;
    Canella::MaterialDescription material_desc;
    Canella::MaterialData new_material;
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

void Canella::AssetSystem::load_material_async( Canella::MaterialDescription material_desc,MaterialCollection& materials ) {

    if(is_material_loaded(material_desc.name,materials))
        return;
    auto load_job = std::make_shared<LoadMaterialFromDisk>(project_src,renderer,material_desc,&materials);
    JobSystem::CanellaJob job(load_job);
    JobSystem::schedule((job));
}
