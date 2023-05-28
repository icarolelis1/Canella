#pragma once
#ifndef VULKAN_RENDER
#define VULKAN_RENDER
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
#include "Resources/Resources.h"
#include "Logger/Logger.hpp"
#include "Window/Window.h"
#include "ComponentBase.h"
#include "RenderGraph/RenderGraph.h"

namespace Canella
{
	namespace RenderSystem
	{
		namespace VulkanBackend
		{
			using Surface = VkSurfaceKHR;

			class VulkanRender : public Render
			{

				Surface surface;
				Instance *instance;
				std::vector<std::shared_ptr<Buffer>> global_buffers;
				Drawables m_drawables;
				RenderGraph render_graph;

				void init_descriptor_pool();
				void init_vulkan_instance();
				void setup_frames();
				void cache_pipelines(const char* pipelines);
				void record_command_index(VkCommandBuffer& commandBuffer, glm::mat4&viewProjection,uint32_t index);
				void allocate_global_usage_buffers();
				void destroy_descriptor_set_layouts();
				void write_global_descriptorsets();
				void destroy_pipeline_layouts();
				void allocate_global_descriptorsets();
                float t = 0;
			public:
                PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT;
                DescriptorSetLayouts cachedDescriptorSetLayouts;
                Pipelines cachedPipelines;
                std::vector<FrameData> frames;
                ResourcesManager resources_manager;

                Device device;
                Descriptorpool descriptorPool;
                Commandpool transfer_pool;
                Swapchain swapChain;
                std::vector<VkDescriptorSet> global_descriptors;
                unsigned int current_frame = 0;
                PipelineLayouts cachedPipelineLayouts;
                Drawables &get_drawables() override;
                std::unique_ptr<RenderpassManager> renderpassManager;

				VulkanRender(nlohmann::json& config, Windowing* window);
				~VulkanRender();
				void render(glm::mat4& viewProjection) override;
				void update(float time) override;
                void enqueue_drawables(Drawables&) override;


            };

		}
	}
}
#endif