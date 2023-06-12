#include "Project/Project.h"
#include "Components/Components.h"
#include "Systems/Systems.h"

using namespace Canella;
Application::Application(Canella::GlfwWindow *_window, Canella::Render *_render):application_time(0){
    window = _window;
    render = _render;
}


void Application::load(nlohmann::json& config)
{
    JobSystem::initialize();
    //Setup Project Folder
    setup_project_folder(config);
    scene = std::make_shared<Scene>(assets_folder,render);
    //serialize scene
    serializer.Serialize(scene, config["Scenes"]);
    scene->init_systems();
}


void Application::setup_project_folder(nlohmann::json& data)
{
    assets_folder = data["Assets"].get<std::string>();
}


void Application::run()
{
    while (playing) {

        float time = (float)glfwGetTime();
        application_time.time = time - application_time.last_time_frame;
        frame_time = application_time.time_in_milli();
        application_time.last_time_frame = time;
        playing = ~window->shouldCloseWindow();
        
        window->update();
        scene->update_systems(frame_time);
        render->render(scene->main_camera->view,scene->main_camera->projection);
        if (KeyBoard::getKeyBoard().getKeyPressed(GLFW_KEY_ESCAPE))
            break;
    }
}

void Application::close()
{
}


Application::~Application()
{
    JobSystem::stop();
    close();
}
