#include "Application/Application.h"
#include "VulkanRender/VulkanRender.h"
#include "JobSystem/JobSystem.h"

using namespace Canella;
void Application::App::initialize(nlohmann::json& config)
{
	Canella::JobSystem::initialize();
	window.initialize(config["Window"]);
	render = std::make_unique<RenderSystem::VulkanBackend::VulkanRender>(config["Render"], &window);
	scenegraph.build(config["Scenegraph"]);
}

void Application::App::run()
{
	while (playing)
	{
		playing = ~window.shouldCloseWindow();
		glfwPollEvents();
		scenegraph.udpate(scenegraph.root,0.f);
		render->update((float)glfwGetTime());

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