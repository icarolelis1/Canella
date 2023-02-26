#include "Application/Application.h"
#include "VulkanRender/VulkanRender.h"
#include "JobSystem/JobSystem.h"
#include <functional>
#include <memory.h>

using namespace Canella;

void Application::App::initialize(nlohmann::json& config)
{
	Canella::JobSystem::initialize();
	window.initialize(config["Window"]);
	render = std::make_unique<Canella::RenderSystem::VulkanBackend::VulkanRender>(config["Render"], &window);
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

		if (Canella::KeyBoard::getKeyBoard().getKeyPressed(GLFW_KEY_ESCAPE))
			break;
	}
}

void Application::App::close()
{
	render.reset();
}

Application::App::~App()
{

	Canella::JobSystem::stop();
	close();
}