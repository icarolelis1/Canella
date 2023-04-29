#include "Application/Application.h"
#include <unordered_map>
using namespace Canella;

void Application::App::initialize(nlohmann::json& config)
{
    ComponentRegistry::getInstance().initializeRegistry();
    JobSystem::initialize();
    window.initialize(config["Window"]);
    scene.build(config["Scene"]);
	ComponentRegistry &registry = ComponentRegistry::getInstance();
    auto drawables = registry.components_map["Mesh"];
    render = std::make_unique<RenderSystem::VulkanBackend::VulkanRender>(
        config["Render"],
        &window, 
        registry.components_map["Mesh"]);
    
}

void Application::App::run()
{
    while (playing)
    {
        playing = ~window.shouldCloseWindow();
        glfwPollEvents();
        render->render();
        render->update(static_cast<float>(glfwGetTime()));
        if (KeyBoard::getKeyBoard().getKeyPressed(GLFW_KEY_ESCAPE))
            break;
    }
}

void Application::App::close()
{
    render.reset();
}

Application::App::~App()
{
    JobSystem::stop();
    close();
}
