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
#define MAX_OBJECTS_TRANSFORM 500
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

                VkCommandBuffer request_command_buffer(VkCommandBufferLevel);
                void render(glm::mat4& view,glm::mat4& projection) override;
                void log_statistic_data(TimeQueryData&) override;
                void enqueue_drawables(Drawables&) override;
                void begin_command_buffer(VkCommandBuffer);
                void end_command_buffer(VkCommandBuffer);
                void set_windowing(Windowing* window);
                void build(nlohmann::json&) override;
                Drawables &get_drawables() override;
                void update(float time) override;
                void destroy();


                Event<VkCommandBuffer&,uint32_t,FrameData&> OnRecordCommandEvent;
#if RENDER_EDITOR_LAYOUT
                std::vector<TimeQueryData*> get_render_graph_timers();
#endif
                Device device;
                Pipelines cachedPipelines;
                PipelineLayouts cachedPipelineLayouts;
                DescriptorSetLayouts cachedDescriptorSetLayouts;
                PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT;
                PFN_vkCmdDrawMeshTasksIndirectEXT vkCmdDrawMeshTasksIndirectEXT;
                RenderpassManager renderpassManager;
                std::vector<FrameData> frames;
                std::vector<VkDescriptorSet> global_descriptors;
                std::vector<VkDescriptorSet> transform_descriptors;
                ResourcesManager resources_manager;
                Descriptorpool descriptorPool;
                Commandpool transfer_pool;
                Swapchain swapChain;
                Instance *instance;
                unsigned int current_frame = 0;

            private:
                Surface surface;
                std::vector<ResourceAccessor> global_buffers;
                std::vector<ResourceAccessor> transform_buffers;
                Drawables m_drawables;
                RenderGraph render_graph;
                Windowing* window;
                Commandpool command_pool;

                void record_command_index(VkCommandBuffer& commandBuffer,uint32_t index);
                void cache_pipelines(const char* pipelines);
                void destroy_descriptor_set_layouts();
                void allocate_global_descriptorsets();
                void allocate_global_usage_buffers();
                void write_global_descriptorsets();
                void create_transform_buffers();
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