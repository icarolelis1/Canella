#include "Application/Application.h"
#include "Components/Components.h"
using namespace Canella;

void Application::App::initialize(nlohmann::json& config)
{
    JobSystem::initialize();
    //Setup Project Folder
    SetupProjectFolder(config);
    //init windowing
    window.initialize(config["Window"]);
    //load scene
    scene = std::make_shared<Scene>();
    //serialize scene
    serializer.Serialize(scene, config["Scenes"]);
    //init render
    render = std::make_unique<RenderSystem::VulkanBackend::VulkanRender>(config["Render"],&window);
}

/**
 * \brief 
 * \param data 
 */
void Application::App::SetupProjectFolder(nlohmann::json& data)
{
    assetsFolder = data["Assets"].get<std::string>();
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
        {
            serializer.Deserialize(scene);
            break;
        }
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
