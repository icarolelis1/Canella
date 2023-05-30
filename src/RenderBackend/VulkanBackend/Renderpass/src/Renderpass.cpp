#include "Renderpass/Renderpass.h"
#include "Resources/Resources.h"

//todo properly extract this class
//todo serialize the creation of framebuffers
namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            RenderPass::RenderPass(Device* _device,
                                   std::string _key,
                                   Swapchain* swapchain,
                                   VkExtent2D _extent,
                                   std::vector<RenderAttachment>& _attachemnts,
                                   std::vector<Subpass>& _subpasses) : device(_device),
                                                                       extent(_extent),
                                                                       key(_key),
                                                                       attachments(_attachemnts),
                                                                       subpasses(_subpasses)
            {
                std::vector<VkAttachmentDescription> descriptions;
                for (auto& d : attachments)
                    descriptions.push_back(d.description);

                VkRenderPassCreateInfo renderPassInfo = {};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                renderPassInfo.pAttachments = descriptions.data();
                renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
                renderPassInfo.pSubpasses = subpasses[0].description.data();
                renderPassInfo.subpassCount = static_cast<uint32_t>(subpasses[0].description.size());
                renderPassInfo.dependencyCount = static_cast<uint32_t>(subpasses[0].dependencies.size());
                renderPassInfo.pDependencies = subpasses[0].dependencies.data();
                renderPassInfo.flags = 0;
                renderPassInfo.pNext = VK_NULL_HANDLE;
                if (vkCreateRenderPass(device->getLogicalDevice(), &renderPassInfo, device->getAllocator(),
                                       &vk_renderpass) != VK_SUCCESS)
                {
                    Canella::Logger::Error("failed to create render pass!");
                }

                create_images(swapchain);
                createFrameBuffer(swapchain);
            };


            VkRenderPass& RenderPass::get_vk_render_pass()
            {
                return vk_renderpass;
            }


            void RenderPass::beginRenderPass(
                VkCommandBuffer& commandBuffer,
                std::vector<VkClearValue> clearValues,
                uint32_t imageIndex,
                VkSubpassContents contents)
            {
                VkRenderPassBeginInfo info = {};
                info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                info.framebuffer = vk_framebuffers[imageIndex];
                info.renderArea.extent.width = extent.width;
                info.renderArea.extent.height = extent.height;
                info.clearValueCount = static_cast<uint32_t>(clearValues.size());
                info.pClearValues = clearValues.data();
                info.renderPass = vk_renderpass;
                vkCmdBeginRenderPass(commandBuffer, &info, contents);
            }

            void RenderPass::createFrameBuffer(Swapchain* swapchain)
            {
                auto targets = swapchain->getViews();
                vk_framebuffers.resize(static_cast<uint32_t>(targets.size()));
                auto i = 0;
                for (auto& view : targets)
                {
                    VkImageView views[2] = {view,image_accessors[i]->view};
                    VkFramebufferCreateInfo fbufCreateInfo{};
                    fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    fbufCreateInfo.renderPass = vk_renderpass;
                    fbufCreateInfo.attachmentCount = 2;
                    fbufCreateInfo.pAttachments =views;
                    fbufCreateInfo.width = extent.width;
                    fbufCreateInfo.height = extent.height;
                    fbufCreateInfo.layers = 1;
                    VkResult r = vkCreateFramebuffer(device->getLogicalDevice(), &fbufCreateInfo,
                                                     device->getAllocator(), &vk_framebuffers[i]);
                    i++;
                }
            }

            //Todo refactor this. Make attachments creation dynamic
            void RenderPass::create_images(Swapchain *swapchain) {
                auto number_of_images = swapchain->getNumberOfImages();
                //todo save this only once
                auto depth_format = device->get_depth_supported_format({ VK_FORMAT_D32_SFLOAT,
                                                                         VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                                         VK_FORMAT_D24_UNORM_S8_UINT },
                                                                       VK_IMAGE_TILING_OPTIMAL,
                                                                       VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
                for(auto i  =0 ; i < number_of_images; ++i)
                {
                    image_accessors.push_back(
                            std::make_shared<Image>(device, extent.width, extent.height,
                                                               depth_format,
                                                                      VK_IMAGE_TILING_OPTIMAL,
                                                                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|
                                                                      VK_IMAGE_USAGE_SAMPLED_BIT ,
                                                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                                      0,
                                                                      VK_IMAGE_ASPECT_DEPTH_BIT));
                }
                //, 1, 0
            }

            void RenderPass::endRenderPass(VkCommandBuffer commandBuffer)
            {
                vkCmdEndRenderPass(commandBuffer);
            }

            RenderPass::~RenderPass()
            {
                vkDestroyRenderPass(device->getLogicalDevice(), vk_renderpass, device->getAllocator());
                 for(auto& img : image_accessors)
                    img.reset();
                 for(auto& frame : vk_framebuffers)
                     vkDestroyFramebuffer(device->getLogicalDevice(),frame,device->getAllocator());
            }


        }
    }
}
