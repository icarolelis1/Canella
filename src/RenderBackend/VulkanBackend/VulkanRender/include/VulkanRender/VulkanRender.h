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
#include "PipelineBuilder/PipelineBuilder.h"
#include <memory.h>
#include <Resources/Resources.h>
#include <vector>
#include "Logger/Logger.hpp"
#include "Window/Window.h"


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
				DescriptorSetLayouts cachedDescriptorSetLayouts;
				PipelineLayouts cachedPipelineLayouts;
				Pipelines cachedPipelines;
				unsigned int current_frame = 0;
				std::vector<VkDescriptorSet> global_descriptors;
				std::vector<Buffer> global_buffers;
				Descriptorpool descriptorPool;
				std::vector<FrameData> frames;
				
				void initDescriptorPool();
				void initPipelines();
				void initVulkanInstance();
				void setupFrames();
				void createPipelineLayouts(const char* pipelines);
				void createPipelines(const char* pipelines);
				void cachePipelines(const char* pipelines);
				void cacheDescriptorSetLayout(nlohmann::json pipelineData, std::vector<DescriptorSetLayout>
					&descriptor_set_layouts, const int i);
				void cacheDescriptorSetLayouts(nlohmann::json pipelineData, std::vector<VkPushConstantRange>& pushConstants);
				void recordCommandIndex(VkCommandBuffer& commandBuffer,uint32_t currentIndex);
				void writeBuffers();
				void destroyDescriptorSetLayouts();
				void destroyPipelineLayouts();

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