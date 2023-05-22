#pragma once
#ifndef DESCRIPTOR_SET
#define DESCRIPTOR_SET
#include <VulkanRender/VulkanRender.h>

//todo recreate this from scratch
namespace Canella::RenderSystem::VulkanBackend
{
    class DescriptorSet
    {
    public:
        DescriptorSet() = default;
        static void update_descriptorset(Device *device, VkDescriptorSet &descriptorset,
                                         std::vector<VkDescriptorBufferInfo> &bufferInfos,
                                         std::vector<VkDescriptorImageInfo> &imageInfos, bool dynamicUbo = 0,
                                         bool storage_buffers = false);

        virtual ~DescriptorSet() = default;

    private:
    };
}


#endif
