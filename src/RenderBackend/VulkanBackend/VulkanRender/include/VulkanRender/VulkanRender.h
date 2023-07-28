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
#define RENDER_EDITOR_LAYOUT 1

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


                struct AsyncTransferJob
                {
                    VkCommandBuffer              cmd         = VK_NULL_HANDLE;
                    uint32_t                     frameSignal = 0;
                    VkSemaphore                  semaphore   = VK_NULL_HANDLE;
                    VkFence                      fence       = VK_NULL_HANDLE;
                    bool                         print       = true;
                    uint64_t                     purgeableResources;
                };

                struct TestTransfer
                {
                    AsyncTransferJob  transfer;
                    Commandpool        transferCmdPool;
                    VkFence            transferFence;
                    VkSemaphore        transferSemaphore;

                };


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
                 * @param display_event Event bound to editor UI that is used to display rendernode statistics and timers
                 */
                void build( nlohmann::json &data, OnOutputStatsEvent* display_event = nullptr) override;
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

                Event<VkCommandBuffer &, uint32_t &> OnRecordCommandEvent;
                PFN_vkCmdDrawMeshTasksEXT vkCmdDrawMeshTasksEXT;
                PFN_vkCmdDrawMeshTasksIndirectEXT vkCmdDrawMeshTasksIndirectEXT;
                PFN_vkCmdDrawMeshTasksIndirectCountEXT vkCmdDrawMeshTasksIndirectCountEXT;
                PFN_vkCmdPushDescriptorSetWithTemplateKHR vkCmdPushDescriptorSetWithTemplateKHR;
                Device device;
                Pipelines cachedPipelines;
                PipelineLayouts cachedPipelineLayouts;
                DescriptorSetLayouts cachedDescriptorSetLayouts;
                bool                 enqueue_new_mesh = false;
                RenderpassManager    renderpassManager;
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
                Drawables drawables_to_be_inserted;
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
                void setup_internal_renderer_events();
                void init_descriptor_pool();
                void init_vulkan_instance();
                void destroy_pipelines();
                void setup_frames();
                int8_t should_reload =  0;
                void get_device_proc();

                void update_view_projection( glm::mat4 &view, glm::mat4 &projection, uint32_t next_image_index );
            };
        }
    }
}
#endif