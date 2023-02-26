#pragma once
#ifndef VULKAN_RENDER
#define VULKAN_RENDER
#include <unordered_map>
#include "Device/Device.h"
#include "Render/Render.h"
#include "Swapchain/Swapchain.h"
#include "Descriptorpool/Descriptorpool.h"
#include "Windowing.h"
#include "RenderpassManager/RenderpassManager.h"
#include "Pipeline/Pipeline.h"
#include "Frame/Frame.h"
#include <memory.h>
namespace Canella
{
	namespace RenderSystem
	{
		namespace VulkanBackend
		{
			using Surface = VkSurfaceKHR;

			class VulkanRender : public Canella::Render
			{

				Device device;
				Swapchain swapChain;
				Surface surface;
				Instance *instance;
				std::unique_ptr<RenderpassManager> renderpassManager;
				Descriptorpool descriptorPool;
				std::vector<FrameData> frames;
				void initDescriptorPool();
				void initPipelines();
				void initVulkanInstance();
				void loadFrames();
				void cachePipelines();
				void recordCommandIndex(VkCommandBuffer& commandBuffer,uint32_t currentIndex);
				unsigned int current_frame = 0;
				std::vector<VkDescriptorSet> global_descriptors;

			public:
				void initialize(Windowing *window);
				void render();

				VulkanRender(nlohmann::json &config, Windowing *window);
				void update(float time);

				~VulkanRender();
			};

		}
	}
}
#endif