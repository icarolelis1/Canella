#pragma once
#ifndef CANELLA_UTILITY
#define CANELLA_UTILITY

#include <vulkan/vulkan.h>

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            VkFormat convert_from_string_format(const char *image_format);
            VkSampleCountFlagBits convert_from_string_sample_count(const char *samples);
            VkAttachmentLoadOp convert_from_string_loadOp(const char *loadOp);
            VkAttachmentStoreOp convert_from_string_storeOp(const char *storeOp);
            VkImageLayout convert_from_string_image_layout(const char *layout);
            VkPipelineStageFlagBits convert_from_string_pipeline_stage(const char* stage);
            VkAccessFlagBits convert_from_string_access_mask(const char* mask);
        }
    }
}
#endif