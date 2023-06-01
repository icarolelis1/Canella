#include "Renderpass/Renderpass.h"
#include "Resources/Resources.h"
#include "CanellaUtility/CanellaUtility.h"


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
                                    std::vector<Subpass>& _subpasses,
                                   ResourcesManager* resource_manager,
                                   nlohmann::json& framebufferRessources
                                   ) : device(_device),
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

                VK_CHECK (vkCreateRenderPass(device->getLogicalDevice(),
                                             &renderPassInfo,
                                             device->getAllocator(),
                                           &vk_renderpass),
                                                     "failed to create render pass!");

                create_images(swapchain,resource_manager,framebufferRessources["ResourcesToCreate"]);

                create_frame_buffer(swapchain,resource_manager,framebufferRessources["FrameBufferAttachments"]);
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

            void RenderPass::create_frame_buffer(Swapchain* swapchain,
                                                 ResourcesManager* resource_manager,
                                                 nlohmann::json& frame_buffers_meta)
            {
                auto targets = swapchain->getViews();
                std::vector<VkImageView> views(frame_buffers_meta.size());
                vk_framebuffers.resize(static_cast<uint32_t>(targets.size()));
                auto i = 0;
                for (auto& view : targets)
                {
                    //Get the image views that the renderpass uses
                    // The field ResourceIndex refers to an image from the images created in create_images
                    for(auto view_index = 0 ; view_index < views.size(); ++view_index){
                        auto img_index = frame_buffers_meta[view_index]["ResourceIndex"].get<std::uint32_t>();
                        //If index -1. It means that it will use the stardard output
                        if( img_index == -1 )
                            views[view_index] = view;
                        else
                            views[view_index] = resource_manager->get_image_cached(image_accessors[img_index][i])->view;
                    }

                    VkFramebufferCreateInfo fbuf_create_info{};
                    fbuf_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    fbuf_create_info.renderPass = vk_renderpass;
                    fbuf_create_info.attachmentCount = static_cast<uint32_t>(views.size());
                    fbuf_create_info.pAttachments =views.data();
                    fbuf_create_info.width = extent.width;
                    fbuf_create_info.height = extent.height;
                    fbuf_create_info.layers = 1;
                    VK_CHECK(vkCreateFramebuffer(
                                     device->getLogicalDevice(),
                                     &fbuf_create_info,
                                     device->getAllocator(),
                                     &vk_framebuffers[i]),
                                     "Failed to create framebuffer!");
                     i++;
                }
            }

            //Todo refactor this. Make attachments creation dynamic
            void RenderPass::create_images(Swapchain *swapchain,
                                           ResourcesManager* resource_manager,
                                           nlohmann::json& images_meta)
            {
                auto image_index = 0;
                image_accessors.resize(images_meta.size());
                //loop  through all the attachments that need to be created for the swapchain
                for(auto& image_meta : images_meta)
                {

                    auto number_of_images = image_meta["NumberOfImages"].get<std::uint32_t>();
                    if(number_of_images == -1) number_of_images = swapchain->getNumberOfImages();
                    auto format_str = image_meta["Format"].get<std::string>();
                    VkFormat format;

                    if(strcmp(format_str.c_str(),"SupportedDepth") == 0)
                        format = device->get_depth_format();
                    for(auto i  =0 ; i < number_of_images; ++i)
                    {
                        //Todo properly refactor this just using the flagsbit
                        //Create the depth image
                        if(!image_meta["ColorImage"].get<bool>())
                            image_accessors[image_index].push_back(
                                resource_manager->create_image(device, extent.width, extent.height,
                                                               format,
                                                               VK_IMAGE_TILING_OPTIMAL,
                                                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|
                                                               VK_IMAGE_USAGE_SAMPLED_BIT ,
                                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                               0,
                                                               VK_IMAGE_ASPECT_DEPTH_BIT));
                        //Create a Color Image
                        else {
                            image_accessors[image_index].push_back(
                                    resource_manager->create_image(device, extent.width, extent.height,
                                                                   format,
                                                                   VK_IMAGE_TILING_OPTIMAL,
                                                                   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|
                                                                   VK_IMAGE_USAGE_SAMPLED_BIT ,
                                                                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                                   0,
                                                                   VK_IMAGE_ASPECT_COLOR_BIT));
                        }


                    }
                    image_index++;
                }
            }

            void RenderPass::endRenderPass(VkCommandBuffer commandBuffer)
            {
                vkCmdEndRenderPass(commandBuffer);
            }

            RenderPass::~RenderPass()
            {
                vkDestroyRenderPass(device->getLogicalDevice(), vk_renderpass, device->getAllocator());
                 for(auto& frame : vk_framebuffers)
                     vkDestroyFramebuffer(device->getLogicalDevice(),frame,device->getAllocator());
            }


        }
    }
}
