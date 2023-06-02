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
#include "Eventsystem/Eventsystem.hpp"

#define RENDER_EDITOR_LAYOUT 1

namespace Canella
{
	namespace RenderSystem
	{
		namespace VulkanBackend
		{
			using Surface = VkSurfaceKHR;

			class VulkanRender : public Render
			{
            public:
                VulkanRender();
                void destroy();
                void set_windowing(Windowing* window);
                void build(nlohmann::json&) override;
                void render(glm::mat4& viewProjection) override;
                void update(float time) override;
                void begin_command_buffer(VkCommandBuffer);
                void end_command_buffer(VkCommandBuffer);
                void enqueue_drawables(Drawables&) override;
                void log_statistic_data(TimeQueryData&) override;
                VkCommandBuffer request_command_buffer(VkCommandBufferLevel);
                Drawables &get_drawables() override;
                Event<VkCommandBuffer&,uint32_t,FrameData&> OnRecordCommandEvent;

#if RENDER_EDITOR_LAYOUT
                std::vector<TimeQueryData*> get_render_graph_timers();
#endif
                Device device;
                Pipelines cachedPipelines;
                PipelineLayouts cachedPipelineLayouts;
                DescriptorSetLayouts cachedDescriptorSetLayouts;
                PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT;
                std::vector<FrameData> frames;
                std::vector<VkDescriptorSet> global_descriptors;
                ResourcesManager resources_manager;
                Descriptorpool descriptorPool;
                Commandpool transfer_pool;
                Swapchain swapChain;
                RenderpassManager renderpassManager;
                Instance *instance;
                unsigned int current_frame = 0;

            private:
                Surface surface;
                std::vector<ResourceAccessor> global_buffers;
                Drawables m_drawables;
                RenderGraph render_graph;
                Windowing* window;
                Commandpool command_pool;

                void record_command_index(VkCommandBuffer& commandBuffer, glm::mat4&viewProjection,uint32_t index);
                void cache_pipelines(const char* pipelines);
                void destroy_descriptor_set_layouts();
                void allocate_global_descriptorsets();
                void allocate_global_usage_buffers();
                void write_global_descriptorsets();
                void destroy_pipeline_layouts();
                void setup_renderer_events();
                void init_descriptor_pool();
                void init_vulkan_instance();
                void destroy_pipelines();
                void setup_frames();
                float t = 0;
            };
        }
	}
}
#endif