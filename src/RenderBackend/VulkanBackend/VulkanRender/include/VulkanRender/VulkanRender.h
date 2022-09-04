#pragma once
#ifndef VULKAN_RENDER
#define VULKAN_RENDER
#include "Device/Device.h"
#include "Render/Render.h"
#include <Windowing.h>
#include "Swapchain/Swapchain.h"

namespace Canella {
	namespace RenderSystem
	{
		namespace VulkanBackend
		{
			using Surface = VkSurfaceKHR;

			class VulkanRender : public Canella::Render {

				Device device;
				Swapchain swapChain;
				Surface surface;
				void initVulkanInstance();
				Instance* instance;

			public:

				void initialize(Windowing* window);
				void render();
				VulkanRender(nlohmann::json& config);
				void update(float time);

				~VulkanRender();
			};

		}
	}
}
#endif