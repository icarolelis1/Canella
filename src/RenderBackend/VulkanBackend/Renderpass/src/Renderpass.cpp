#include "Renderpass/Renderpass.h"

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            RenderPass::RenderPass(Device *_device, 
                const char *_key, 
                VkExtent2D _extent, 
                std::vector<RenderAttachment> &_attachemnts,
                std::vector<Subpass> &_subpasses) :
                device(_device), 
                extent(_extent), 
                key(_key),
                attachments(_attachemnts),
                subpasses(_subpasses)
            {
                std::vector<VkAttachmentDescription> descriptions;
                for (auto &d : attachments)
                    descriptions.push_back(d.description);

                VkRenderPassCreateInfo renderPassInfo = {};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                renderPassInfo.pAttachments = descriptions.data();
                renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
                renderPassInfo.pSubpasses = subpasses[0].description.data();
                renderPassInfo.subpassCount = subpasses[0].description.size();
                renderPassInfo.dependencyCount = static_cast<uint32_t>(subpasses[0].dependencies.size());
                renderPassInfo.pDependencies = subpasses[0].dependencies.data();
                renderPassInfo.flags = 0;
                renderPassInfo.pNext = VK_NULL_HANDLE;
                if (vkCreateRenderPass(device->getLogicalDevice(), &renderPassInfo, device->getAllocator(), &renderPass) != VK_SUCCESS)
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