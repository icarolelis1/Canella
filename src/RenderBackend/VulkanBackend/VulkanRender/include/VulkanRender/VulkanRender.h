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
#include "EntityBase.h"
#include "ComponentBase.h"
#include <unordered_map>

namespace Canella
{
	namespace RenderSystem
	{
		namespace VulkanBackend
		{
	
			using Surface = VkSurfaceKHR;

			class VulkanRender : public Render
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
				Commandpool transfer_pool;
				unsigned int current_frame = 0;
				Drawables m_drawables;
				
				void init_descriptor_pool();
				//void initPipelines();
				void init_vulkan_instance();
				void setup_frames();
				void cache_pipelines(const char* pipelines);
				void cacheDescriptorSetLayouts(nlohmann::json& pipelineData, std::vector<VkPushConstantRange>& pushConstants);
				void record_command_index(VkCommandBuffer& commandBuffer, glm::mat4&
					viewProjection,uint32_t index);
				void allocate_global_usage_buffers();
				void destroy_descriptor_set_layouts();
				void write_global_descriptorsets();
				void destroy_pipeline_layouts();
				void allocate_global_descriptorsets();
				void writeDescriptorSets();
				void create_meshlets_buffers();
				std::vector<MeshletGPUResources> meshlet_gpu_resources;				
			public:
				void enqueue_drawables(Drawables&);
				VulkanRender(nlohmann::json& config, Windowing* window);
				~VulkanRender();
				void update(float time);
				void render(glm::mat4& viewProjection);
			};

		}
	}
}
#endif