#include "Application/Application.h"

Application::App::App(){};
void Application::App::initialize(nlohmann::json &config)
{

    // Initialize Window
    window.initialize(config["Window"]);
    // Initialize Rebder
    render = RenderSystem::Render::createRender(config["Render"]);

    // Scenegraph
    scenegraph = std::make_unique<Engine::Scenegraph>(config["Scenegraph"]["Entities"]);

}
void Application::App::run()
{
    while (playing)
    {

        playing = ~window.shouldCloseWindow();
        // Poll Events
        glfwPollEvents();

        // Run Engine Stuff
        if (Engine::KeyBoard::keyboard_instance->getKeyPressed(GLFW_KEY_ESCAPE))
        {
            break;
        };
    }
}

void Application::App::close()
{
}

Application::App::~App()
{
    close();
}