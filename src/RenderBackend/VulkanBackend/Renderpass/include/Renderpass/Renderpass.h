#pragma once
#ifndef RENDER_PASS

#include "Device/Device.h"
#include "Resources/Resources.h"
#include "Swapchain/Swapchain.h"
#include <vector>
namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            class Image;
            struct RenderAttachment
            {
                VkAttachmentDescription description;
                VkAttachmentReference reference;
            };
            struct Subpass
            {
                std::vector<VkSubpassDependency> dependencies;
                std::vector<VkSubpassDescription> description;
            };
            class RenderpassManager;
            class RenderPass
            {
                Device *device;
                VkRenderPass vk_renderpass;
                std::string key; // String identifier for this renderpass
                VkExtent2D extent;
                std::vector<RenderAttachment> attachments;
                std::vector<Subpass> subpasses;


            public:
                /**
                 * @brief Creates a Renderpass
                 * @param device Vulkan Device wrapper
                 * @param key string identifier of the renderpass
                 * @param swapchain Swapchain
                 * @param extent extent of the renderpass {dimensions width, height}
                 * @param attachments structure that defines a list of VkAttachmentDescription and VkAttachmentReference
                 * @param subpasses structure that defines a list of VkSubpassDependency and VkSubpassDescription
                 * @param resource_manager resource manager of the renderer
                 */
                RenderPass(Device *device,
                    std::string key,
                    Swapchain* swapchain, 
                    VkExtent2D extent, 
                    std::vector<RenderAttachment> &attachments, 
                    std::vector<Subpass> &subpasses,
                    ResourcesManager* resource_manager,
                    nlohmann::json& framebufferRessources,
                    RenderpassManager& renderpass_manager);
                ~RenderPass();

                /**
                 * @brief creates the framebuffer for the swapchain
                 * @param swapchain
                 * @param resource_manager
                 */
                void create_frame_buffer(Swapchain* swapchain,
                                         ResourcesManager* resource_manager,
                                         nlohmann::json& frame_buffers_meta,
                                         RenderpassManager& renderpass_manager);

                /**
                 * @brief creates the image output attachments for the renderpass
                 * @param swapchain application swapchain
                 * @param resource_manager
                 * @param images_meta
                 */
                void create_images(Swapchain* swapchain,ResourcesManager* resource_manager,nlohmann::json& images_meta);

                std::vector<VkFramebuffer> vk_framebuffers;
                std::vector<std::vector<ResourceAccessor>> image_accessors;

                /**
                 * @brief return VkRenderpass object
                 */

                VkRenderPass& get_vk_render_pass();
                /**
                 * @brief internally begins the renderpass
                 *
                 * @param commandBuffer the command to be recorded with beginrenderpass
                 * @param frameBuffer vulkan framebuffer associated with this renderpass
                 * @param contents
                 */
                void beginRenderPass(
                    VkCommandBuffer &commandBuffer,
                    std::vector<VkClearValue> clearValue,
                    uint32_t imageIndex,
                    VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

                /**
                 * @brief calls endRenderpass
                 *
                 * @param cmd the command in which the renderpass has begun
                 */
                void endRenderPass(VkCommandBuffer cmd);

            };
        }
    }

}

#endif