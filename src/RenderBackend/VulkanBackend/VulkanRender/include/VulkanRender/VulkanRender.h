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

#define MAX_OBJECTS_TRANSFORM 500
#define RENDER_EDITOR_LAYOUT 0

namespace Canella
{
    namespace RenderSystem
    {
        // Todo Refactor/Break this class into separate objects
        namespace VulkanBackend
        {
            using Surface = VkSurfaceKHR;

            class VulkanRender : public Render
            {
            public:
                /**
                 * @brief Constructs a vulkan Renderer
                 */
                VulkanRender();
                /**
                 * @brief Use the command pool to allocate a vkCommandBufer
                 * @param level whether its a primary or a secondary command buffer
                 */
                VkCommandBuffer request_command_buffer(VkCommandBufferLevel level);
                /**
                 * @brief Execute the nodes in the rendergraph
                 * @param view view matrix
                 * @param projectin projection matrix
                 */
                void render(glm::mat4 &view, glm::mat4 &projection) override;
                /**
                 * @brief Add a new drawable object to be rendered
                 * @param drawable object added to be rendered
                 */
                void enqueue_drawables(Drawables &drawables) override;
                /**
                 * @brief Call vkBeginCommandBuffer on a command buffer
                 * @param commandbuffer
                 */
                void begin_command_buffer(VkCommandBuffer commandbuffer);
                /**
                 * @brief command Ends the command buffer
                 * @param commandbuffer
                 */
                void end_command_buffer(VkCommandBuffer command);
                /**
                 * @brief passes a pointer to the windowing system used in swapchain creation
                 * @param windowing 
                 */
                void set_windowing(Windowing *window);

                /**
                 * @brief Creates the vulkan renderer and initialize dependencies
                 * @param data metadata containing information about how to build pipelines/descriptors etc
                 */
                void build(nlohmann::json &data) override;
                /**
                 * @brief Return all the drawables enqueued
                 */
                Drawables &get_drawables() override;
                /**
                 * @brief Enqueue a semaphore to be waited before submitting graphics comands
                 * @param semaphore the semaphore to wait
                 */
                void enqueue_waits(VkSemaphore semaphore);
                /**
                 * @brief Add a semaphore to wait before submiting graphics commands
                 * @param semaphore the semaphore to wait
                 */
                void destroy();
                /**
                 * @brief enqueue a new Mesh to be rendered
                 * @param mesh mesh to be rendered
                */
                void enqueue_drawable(ModelMesh& mesh) override;

                Event<VkCommandBuffer &, uint32_t, FrameData &> OnRecordCommandEvent;
#if RENDER_EDITOR_LAYOUT
                std::vector<TimeQueries *> get_render_graph_timers();
#endif
                PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT;
                PFN_vkCmdDrawMeshTasksIndirectEXT vkCmdDrawMeshTasksIndirectEXT;
                PFN_vkCmdDrawMeshTasksIndirectCountEXT vkCmdDrawMeshTasksIndirectCountEXT;
                PFN_vkCmdPushDescriptorSetWithTemplateKHR vkCmdPushDescriptorSetWithTemplateKHR;
                Device device;
                Pipelines cachedPipelines;
                PipelineLayouts cachedPipelineLayouts;
                DescriptorSetLayouts cachedDescriptorSetLayouts;
                bool dispatch_on_enqueue = false;
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
                RenderCameraData render_camera_data;

            private:
                Surface surface;
                std::vector<ResourceAccessor> global_buffers;
                std::vector<ResourceAccessor> transform_buffers;
                Drawables m_drawables;
                RenderGraph render_graph;
                Windowing *window;
                Commandpool command_pool;
                std::vector<VkSemaphore> queued_semaphores;

                void record_command_index(VkCommandBuffer &commandBuffer, uint32_t index);
                void cache_pipelines(const char *pipelines);
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
            };
        }
    }
}
#endif