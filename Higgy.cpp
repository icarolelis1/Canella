#include <iostream>
#include "Render/Render.h"
#include <vulkan/vulkan.h>
#include <json.hpp>
#include <fstream>
#include <Window/Window.h>
int main()
{
	//Load Config File
	
	nlohmann::json j;
	
	RenderSystem::RenderConfig cofig;
	RenderSystem::Render::createRender(j);

	Engine::Window w;
	w.initialize(j);
	return 0;
}
