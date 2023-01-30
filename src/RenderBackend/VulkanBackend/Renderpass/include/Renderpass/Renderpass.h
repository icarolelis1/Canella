#pragma once
#ifndef RENDER_PASS

#include "Device/Device.h"
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
                VkRenderPass renderPass;
                const char *key; // String identifier for this renderpass
                VkExtent2D extent;
                std::vector<VkClearValue> clearValues;
                std::vector<RenderAttachment> attachments;
                Subpass subpass;

            public:
                RenderPass(Device *device, const char *key, VkExtent2D extent, std::vector<RenderAttachment> &attachments, std::vector<Subpass> &subpasses);

                /**
                 * @brief internally begins the renderpass
                 *
                 * @param commandBuffer the command to be recorded with beginrenderpass
                 * @param frameBuffer vulkan framebuffer associated with this renderpass
                 * @param contents
                 */
                void beginRenderPass(VkCommandBuffer &commandBuffer, VkFramebuffer &frameBuffer, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);

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