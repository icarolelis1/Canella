#include "Project/Project.h"
#include "AssetSystem/AssetSystem.h"
#include "Systems/Systems.h"

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
}

void Application::setup_project_folder(nlohmann::json &data)
{
    auto &asset_system = AssetSystem::instance();
    asset_system.set_renderer(render);
    assets_folder = data["Assets"].get<std::string>();
    asset_system.set_project_src(assets_folder.c_str());
}

void Application::run()
{
    float time = (float)glfwGetTime();
    application_time.time = time - application_time.last_time_frame;
    scene->update_systems(frame_time);
    render->render(scene->main_camera->view, scene->main_camera->projection);
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
