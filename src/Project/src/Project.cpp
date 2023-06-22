#include "Project/Project.h"
#include "Components/Components.h"
#include "Systems/Systems.h"
#include "AssetSystem/AssetSystem.h"


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
    scene = std::make_shared<Scene>(assets_folder, render);
    // serialize scene
    serializer.Serialize(scene, config["Scenes"]);
    scene->init_systems();
}

void Application::setup_project_folder(nlohmann::json &data)
{
    auto& asset_system = AssetSystem::instance();
    assets_folder = data["Assets"].get<std::string>();
    asset_system.set_project_src(assets_folder.c_str());
}

void Canella::Application::submit_loaded_model(ModelMesh &mesh)
{
    render->enqueue_drawable(mesh);
}

void Application::run()
{
    float time = (float)glfwGetTime();
    application_time.time = time - application_time.last_time_frame;
    frame_time = application_time.time_in_milli();
    application_time.last_time_frame = time;
    scene->update_systems(frame_time);
    render->render(scene->main_camera->view, scene->main_camera->projection);
}

void Application::close()
{
}

Application::~Application()
{
    JobSystem::stop();
    close();
}
