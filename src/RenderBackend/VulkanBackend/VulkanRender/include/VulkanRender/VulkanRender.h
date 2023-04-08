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
#include "DescriptorSet/DescriptorSet.h"
#include "Frame/Frame.h"
#include "PipelineBuilder/PipelineBuilder.h"
#include <memory.h>
#include <Resources/Resources.h>
#include <vector>
#include "Logger/Logger.hpp"
#include "Window/Window.h"
#include "vulkan/vulkan_core.h"
namespace Canella
{
	namespace RenderSystem
	{
		namespace VulkanBackend
		{
	
			using Surface = VkSurfaceKHR;

			class VulkanRender : public Canella::Render
			{
				PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT;

				Device device;
				Swapchain swapChain;
				Surface surface;
				Instance *instance;
				std::unique_ptr<RenderpassManager> renderpassManager;
				DescriptorSetLayouts cachedDescriptorSetLayouts;
				PipelineLayouts cachedPipelineLayouts;
				Pipelines cachedPipelines;
				std::vector<VkDescriptorSet> global_descriptors;
				std::vector<Buffer> global_buffers;
				Descriptorpool descriptorPool;
				std::vector<FrameData> frames;
				unsigned int current_frame = 0;
				
				void init_descriptor_pool();
				void initPipelines();
				void initVulkanInstance();
				void setup_frames();
				void cache_pipelines(const char* pipelines);
				void cacheDescriptorSetLayouts(nlohmann::json& pipelineData, std::vector<VkPushConstantRange>& pushConstants);
				void record_command_index(VkCommandBuffer& commandBuffer,uint32_t currentIndex);
				void allocateGlobalUsageBuffers();
				void destroyDescriptorSetLayouts();
				void writeGlobalDescriptorsets();
				void destroyPipelineLayouts();
				void allocateGlobalDescriptorsets();
				void writeDescriptorSets();
			public:
				void render();
				VulkanRender(nlohmann::json &config, Windowing *window);
				void assignMainCamera();
				void update(float time);
				~VulkanRender();
			};

		}
	}
}
#endif