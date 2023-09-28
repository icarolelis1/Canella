#include "Project/Project.h"
#include "AssetSystem/AssetSystem.h"
#include "Systems/Systems.h"
#include "VulkanRender/VulkanRender.h"

using namespace Canella;
Application::Application(Canella::GlfwWindow *_window, Canella::Render *_render) : application_time(0)
{
    window = _window;
    render = _render;
}

void Application::load(nlohmann::json &config)
{
    JobSystem::initialize();
    // Setup Project Folder
    setup_project_folder(config);
    //Create the application Scene
    scene = std::make_shared<Scene>();
    // serialize scene
    serializer.Serialize(scene, config["Scenes"],assets_folder);

    //Enqueue initial meshes to render
    std::vector<ModelMesh> meshes;
    load_initial_meshes_on_scene( meshes, scene.get());
    render->enqueue_drawables(meshes);

    //Load materials
    load_initial_materials_on_scene( scene.get(),render,material_collection );
    scene->init_scene();

    //Create Render Graph Resources
    render->create_render_graph_resources();

    JobSystem::wait();

}

void Application::setup_project_folder(nlohmann::json &data)
{
    auto &asset_system = AssetSystem::instance();
    asset_system.set_renderer(render);
    assets_folder = data["Assets"].get<std::string>();
    asset_system.set_project_src(assets_folder.c_str());

    auto irradiance_map = asset_system.load_ktx_cube_map("diffuse_out.ktx2");
    auto specular_map   = asset_system.load_ktx_cube_map("specular_out.ktx2");

    EnvironmentMaps environment_maps;
    environment_maps.irradiance = irradiance_map;
    environment_maps.specular = specular_map;
#if USE_VULKAN
    ((RenderSystem::VulkanBackend::VulkanRender*)render)->set_environment_maps(environment_maps);
#endif
}

void Application::run()
{
    float time = (float)glfwGetTime();
    application_time.time = time - application_time.last_time_frame;
    scene->update_systems(frame_time);
    render->render(scene->main_camera->view,scene->main_camera->entity_transform->position, scene->main_camera->projection);
    auto v = scene->main_camera->view;
    frame_time = application_time.time_in_milli();
    application_time.last_time_frame = time;
}

Application::~Application()
{
    JobSystem::stop();
}

void Application::Deserialize() {
    serializer.Deserialize(scene);
}
