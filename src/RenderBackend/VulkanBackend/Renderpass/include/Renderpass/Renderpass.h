#pragma once
#ifndef RENDER_PASS

#include "Device/Device.h"
#include "Swapchain/Swapchain.h"
#include <vector>
namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
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
            class RenderPass
            {

                Device *device;
                VkRenderPass vk_renderpass;
                std::string key; // String identifier for this renderpass
                VkExtent2D extent;
                std::vector<RenderAttachment> attachments;
                std::vector<Subpass> subpasses;


            public:
                std::vector<VkFramebuffer> vk_framebuffers;
                RenderPass(Device *device, 
                    std::string key,
                    Swapchain* swapchain, 
                    VkExtent2D extent, 
                    std::vector<RenderAttachment> &attachments, 
                    std::vector<Subpass> &subpasses);

                void createFrameBuffer(Swapchain* swapchain);

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

                ~RenderPass();
            };
        }
    }

}

#endif