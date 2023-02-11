#include "CanellaUtility/CanellaUtility.h"

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            VkFormat convert_from_string_format(const char* image_format){
                return VK_FORMAT_B8G8R8A8_UNORM;
            }

            VkSampleCountFlagBits convert_from_string_sample_count(const char* samples) {
                return VK_SAMPLE_COUNT_1_BIT;
            }

            VkAttachmentLoadOp convert_from_string_loadOp(const char* loadOp) {
                return VK_ATTACHMENT_LOAD_OP_CLEAR;
            }

            VkAttachmentStoreOp convert_from_string_storeOp(const char* storeOp) {
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }
            VkImageLayout convert_from_string_image_layout(const char* layout){
                if (layout == "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR") return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                else if (layout == "VK_IMAGE_LAYOUT_UNDEFINED") return VK_IMAGE_LAYOUT_UNDEFINED;
                return VK_IMAGE_LAYOUT_UNDEFINED;
            }
            VkPipelineStageFlagBits convert_from_string_pipeline_stage(const char* stage){

                return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            }
            VkAccessFlagBits convert_from_string_access_mask(const char* mask){
                if(mask == "VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT")return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            }
        }
    }
}