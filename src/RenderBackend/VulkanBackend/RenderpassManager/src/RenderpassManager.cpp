#include "RenderpassManager/RenderpassManager.h"
#include "Logger/Logger.hpp"
#include "Renderpass/Renderpass.h"
#include "CanellaUtility/CanellaUtility.h"
#include <fstream>

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            RenderpassManager::RenderpassManager(Device* _device, Swapchain* swapchain,
                                                 const char* render_path) : device(_device)
            {
                // Read renderpath json
                std::fstream f(render_path);
                f;
                nlohmann::json j;
                f >> j;

                uint32_t number_of_passes = j["Passes"]["Number_of_passes"].get<std::uint32_t>();
                Logger::Debug("Number of Renderpasses %d", number_of_passes);

                // Object that contains the description of all render passes defined in the render_path file
                RenderpassManagerDescription renderpassManagerDescription;

                // Number of passes
                renderpassManagerDescription.number_of_passes = number_of_passes;
                renderpassManagerDescription.renderpasses_descriptions.resize(number_of_passes);

                for (uint32_t i = 0; i < number_of_passes; ++i)
                {
                    nlohmann::json renderpassJson = j["Passes"]["Renderpass"][i];

                    uint32_t attchmenCount = renderpassJson["Attachment_count"].get<std::uint32_t>();

                    // Resize to number of attachments
                    renderpassManagerDescription.renderpasses_descriptions[i].attachements.resize(attchmenCount);

                    // Read  how many internal subpasses this particualr Renderpass at index i has
                    uint32_t subpasses_count = renderpassJson["Subpasses_count"].get<std::uint32_t>();
                    renderpassManagerDescription.renderpasses_descriptions[i].number_of_subpasses = subpasses_count;
                    renderpassManagerDescription.renderpasses_descriptions[i].subpasses.resize(subpasses_count);

                    // Vector of renderpass attachments
                    std::vector<RenderAttachment> render_attachments_descriptions;

                    // For each attachemnt defined in the renderpass i we store the vulkan especification for it
                    for (uint32_t j = 0; j < attchmenCount; ++j)
                    {
                        nlohmann::json renderAttachmentDescriptionJson = renderpassJson["Attachments_descriptions"][
                            std::to_string(j)];
                        RenderAttachment attachment;
                        attachment.description = {};
                        attachment.reference = {};
                        std::string format_str = renderAttachmentDescriptionJson["Format"].get<std::string>();
                        VkFormat format = get_attachment_format(format_str.c_str(),swapchain);
                        attachment.description.format = format;
                        attachment.description.samples = convert_from_string_sample_count(
                            renderAttachmentDescriptionJson["Samples"].get<std::string>().c_str());
                        attachment.description.loadOp = convert_from_string_loadOp(
                            renderAttachmentDescriptionJson["LoadOp"].get<std::string>().c_str());
                        attachment.description.storeOp = convert_from_string_storeOp(
                            renderAttachmentDescriptionJson["StoreOp"].get<std::string>().c_str());
                        attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                        attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        attachment.description.initialLayout = convert_from_string_image_layout(
                            renderAttachmentDescriptionJson["InitialLayout"].get<std::string>().c_str());
                        attachment.description.finalLayout =
                                convert_from_string_image_layout(
                                        renderAttachmentDescriptionJson["FinalLayout"].get<std::string>().c_str());
                        attachment.description.flags = 0;
                        attachment.reference.attachment = renderAttachmentDescriptionJson["Attachment"].get<
                            std::uint32_t>();

                        attachment.reference.layout = convert_from_string_image_layout(
                                renderAttachmentDescriptionJson["AttachmentLayout"].get<std::string>().c_str());
                        render_attachments_descriptions.push_back(attachment);
                    }

                    // For each internal subpass in the renderpass of index i we store the subpassdescription and dependency for it
                    std::vector<Subpass> subpasses;
                    for (uint32_t j = 0; j < renderpassManagerDescription.renderpasses_descriptions[i].
                         number_of_subpasses; ++j)
                    {
                        // Object that holds subpass description and dependencies
                        Subpass subpass;
                        nlohmann::json subpassesDescriptionsJson = renderpassJson["Subpasses_descriptions"];
                        subpass.description.resize(subpassesDescriptionsJson["Subpasses_count"].get<std::uint32_t>());

                        // For each subpass description inside subpass j
                        VkSubpassDescription description = {};
                        for (uint32_t x = 0; x < subpass.description.size(); ++x)
                        {
                            nlohmann::json subpassDescriptionJson = subpassesDescriptionsJson[std::to_string(x)];

                            description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

                            description.colorAttachmentCount = subpassDescriptionJson["ColorAttachmentCount"].get<
                                std::uint32_t>();
                            description.inputAttachmentCount = 0;
                            description.preserveAttachmentCount = 0;
                            description.pColorAttachments = &render_attachments_descriptions[subpassDescriptionJson[
                                "ColorAttachemntIndex"][0].get<std::uint32_t>()].reference;
                            bool has_depth = subpassDescriptionJson["HasDepthAttachment"].get<bool>();
                            if(has_depth)
                            {
                                description.pDepthStencilAttachment = &render_attachments_descriptions[subpassDescriptionJson[
                                        "DepthAttachmentIndex"].get<std::uint32_t>()].reference;
                            }
                            description.flags = 0;
                            subpass.description[x] = description;
                            VkSubpassDependency dependency = {};
                            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                            dependency.dstSubpass = 0;
                            dependency.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                            dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                            subpass.dependencies.push_back(dependency);
                            Canella::Logger::Info("testing stuff");
                        }
                        subpasses.push_back(subpass);
                    }
                    // Move the vector of attachment description to the renderpassmanager
                    renderpassManagerDescription.renderpasses_descriptions[i].attachements = std::move(
                        render_attachments_descriptions);
                    renderpassManagerDescription.renderpasses_descriptions[i].subpasses = std::move(subpasses);

                    VkExtent2D extent;
                    extent.width = renderpassJson["Extent"]["Width"].get<std::uint32_t>();
                    extent.height = renderpassJson["Extent"]["Height"].get<std::uint32_t>();
                    auto key = renderpassJson["Key"].get<std::string>();
                    loadRenderPassManager(key, swapchain, extent, renderpassManagerDescription);
                }
            }

            void RenderpassManager::loadRenderPassManager(std::string key, Swapchain* swapchain, VkExtent2D extent,
                                                          RenderpassManagerDescription& managerdescription)
            {
                renderpasses[key] = new RenderPass(device, key, swapchain, extent,
                                                   managerdescription.renderpasses_descriptions[0].attachements,
                                                   managerdescription.renderpasses_descriptions[0].subpasses);
            }

            VkFormat RenderpassManager::get_attachment_format(const char *format_str,Swapchain* swapchain) {
                if(strcmp(format_str,"SUPPORTED_DEPTH") == 0)
                    return device->get_depth_supported_format({ VK_FORMAT_D32_SFLOAT,
                                                                VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                                VK_FORMAT_D24_UNORM_S8_UINT },
                                                                VK_IMAGE_TILING_OPTIMAL,
                                                                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
                if(strcmp(format_str,"SWAPCHAIN") == 0)
                    return swapchain->getFormat();
                return swapchain->getFormat();
            }


            void RenderpassManager::destroy_renderpasses() {
                renderpasses;
                auto it = renderpasses.begin();
                while(it != renderpasses.end()){
                    delete(it->second);
                    it++;
                }
            }
        }
    }
}
