#include "Application/Application.h"

#include <ComponentRegistry/ComponentRegistry.h>

#include "VulkanRender/VulkanRender.h"
#include "JobSystem/JobSystem.h"

using namespace Canella;
void Application::App::initialize(nlohmann::json& config)
{
	ComponentRegistry::getInstance().initializeRegistry();
	JobSystem::initialize();
	window.initialize(config["Window"]);
	render = std::make_unique<RenderSystem::VulkanBackend::VulkanRender>(config["Render"], &window);
	//scenegraph.build(config["Scenegraph"]);
}

void Application::App::run()
{
	while (playing)
	{
		playing = ~window.shouldCloseWindow();
		glfwPollEvents();
		
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