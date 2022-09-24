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

            RenderpassManager::RenderpassManager(Device *device, Swapchain *swapchain, const char *render_path)
            {

                // Read renderpath jason
                std::fstream f(render_path);
                f;
                nlohmann::json j;
                f >> j;

                uint32_t number_of_passes = j["Passes"]["Number_of_passes"].get<std::uint32_t>();
                Logger::Debug("Number of Renderpasses %d", number_of_passes);

                RenderpassManagerDescription renderpassManagerDescription;

                renderpassManagerDescription.renderpasses_descriptions.resize(1);
                renderpassManagerDescription.number_of_passes = number_of_passes;
                for (uint32_t i = 0; i < number_of_passes; ++i)
                {
                    VkFormat format = swapchain->getFormat();
                    renderpassManagerDescription.renderpasses_descriptions[i].attachements.resize(renderpassManagerDescription.renderpasses_descriptions[i].number_of_attachemnts);

                    /**
                     * @brief creates each attachemnt description for this renderpass
                     *
                     */
                    std::vector<RenderAttachment> render_attachments_descriptions;

                    for (uint32_t j = 0; j < renderpassManagerDescription.renderpasses_descriptions[i].number_of_attachemnts; ++j)
                    {
                        RenderAttachment attachment;
                        attachment.description.format = convert_from_string_format("");
                        attachment.description.samples = convert_from_string_sample_count("");
                        attachment.description.loadOp = convert_from_string_loadOp("");
                        attachment.description.storeOp = convert_from_string_storeOp("");
                        attachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                        attachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        attachment.description.initialLayout = convert_from_string_image_layout("");
                        attachment.description.finalLayout = convert_from_string_image_layout("");
                        attachment.description.flags = 0;
                        attachment.reference.attachment = i;
                        attachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        render_attachments_descriptions.push_back(attachment);
                    }
                    /**
                     * @brief creates each subpasses description for this renderpass
                     *
                     */

                    for (uint32_t j = 0; j < renderpassManagerDescription.renderpasses_descriptions[i].number_of_subpasses; ++j)
                    {
                        Subpass subpass;

                        for (uint32_t x = 0; x < subpass.description.size(); ++x)
                        {
                            VkSubpassDescription description;
                            description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                            description.colorAttachmentCount = 1;
                            description.pColorAttachments
                        }
                    }

                    // RenderAttachment renderpassAttachment;
                    // renderpassAttachment.description.format = format;
                    // renderpassAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
                    // renderpassAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    // renderpassAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                    // renderpassAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                    // renderpassAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                    // renderpassAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                    // renderpassAttachment.description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    // renderpassAttachment.description.flags = 0;

                    // renderpassAttachment.reference.attachment = 0;
                    // renderpassAttachment.reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                    Subpass subpass;
                    subpass.description.resize(1);
                    subpass.dependencies.resize(1);

                    subpass.description[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                    subpass.description[0].colorAttachmentCount = 1;
                    subpass.description[0].pColorAttachments = &renderpassAttachment.reference;

                    subpass.dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
                    subpass.dependencies[0].dstSubpass = 0;
                    subpass.dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    subpass.dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    subpass.dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    subpass.dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                    // renderpassProperties.attachments[0] = renderpassAttachment;

                    // swapChainRenderpass->properties = renderpassProperties;
                    // swapChainRenderpass->subpass = subpass;
                }
            }
            void RenderpassManager::loadRenderPassManager(RenderpassManagerDescription &managerdescription)
            {
            }
        }
    }
}