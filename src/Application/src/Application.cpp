#include "Application/Application.h"
#include "VulkanRender/VulkanRender.h"
#include "JobSystem/JobSystem.h"

int finished = 0;
void heavy_work_load()
{
	using namespace std::this_thread; // sleep_for, sleep_until
	using namespace std::chrono;	  // nanoseconds, system_clock, seconds

	sleep_for(nanoseconds(100));
	sleep_until(system_clock::now() + seconds(1));
	finished++;
}

void Application::App::initialize(nlohmann::json &config)
{

	// Initialize GlfwWindow
	// Initialize Rebder
	Canella::JobSystem::initialize();
	render = std::make_unique<Canella::RenderSystem::VulkanBackend::VulkanRender>(config["Render"]);

	window.initialize(config["Window"]);
	render->initialize(&window);

	for (int i = 0; i < 100; i++)
	{

		Canella::JobSystem::execute([]()
									{
										heavy_work_load();
									});
	}

	// Scenegraph

	Canella::JobSystem::wait();
	Canella::Logger::Info("Worked %d", finished);
	
	// Logger
	Canella::Logger::Info("Application initialized");

}

void Application::App::run()
{
	while (playing)
	{

		playing = ~window.shouldCloseWindow();
		// Poll Events
		glfwPollEvents();

		// Run Canella Stuff

		render->update((float)glfwGetTime());

		render->render();

		if (Canella::KeyBoard::getKeyBoard().getKeyPressed(GLFW_KEY_ESCAPE))
		{
			break;
		};
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