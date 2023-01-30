#include "Renderpass/Renderpass.h"

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            RenderPass::RenderPass(Device *_device, const char *_key, VkExtent2D _extent, std::vector<RenderAttachment> &_attachemnts,std::vector<Subpass> &subpasses) : device(_device), extent(_extent), key(_key)
            {

                std::vector<VkAttachmentDescription> descriptions;

                for (auto &d : attachments)
                    descriptions.push_back(d.description);

                VkRenderPassCreateInfo renderPassInfo = {};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                renderPassInfo.pAttachments = descriptions.data();
                renderPassInfo.subpassCount = static_cast<uint32_t>(subpass.description.size());
                renderPassInfo.pSubpasses = subpass.description.data();
                renderPassInfo.dependencyCount = static_cast<uint32_t>(subpass.dependencies.size());
                ;
                renderPassInfo.pDependencies = subpass.dependencies.data();

                if (vkCreateRenderPass(device->getLogicalDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
                {
                    Canella::Logger::Error("failed to create render pass!");
                }
            };

            void RenderPass::RenderPass::beginRenderPass(VkCommandBuffer &commandBuffer, VkFramebuffer &frameBuffer, VkSubpassContents contents)
            {

                VkRenderPassBeginInfo info = {};
                info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                info.renderPass = renderPass;
                info.framebuffer = frameBuffer;
                info.renderArea.extent.width = extent.width;
                info.renderArea.extent.height = extent.height;
                info.clearValueCount = static_cast<uint32_t>(clearValues.size());

                info.pClearValues = clearValues.data();

                vkCmdBeginRenderPass(commandBuffer, &info, contents);
            }
            RenderPass::~RenderPass()
            {
                vkDestroyRenderPass(device->getLogicalDevice(), renderPass, device->getAllocator());
            }
        }
    }
}