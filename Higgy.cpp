#include <iostream>
#include "Render/Render.h"
#include <vulkan/vulkan.h>
#include <json.hpp>
int main()
{
	nlohmann::json j;
	RenderSystem::RenderConfig cofig;
	RenderSystem::Render::createRender(j);

	return 0;
}
