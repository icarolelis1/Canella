#pragma once
#ifndef DESCRIPTOR_SET
#define DESCRIPTOR_SET
#include <VulkanRender/VulkanRender.h>

namespace Canella::RenderSystem::VulkanBackend
{
    class DescriptorSet
    {
    public:
        static void updateDescriptorset(Device* device, VkDescriptorSet& descriptorset,std::vector<VkDescriptorBufferInfo>& bufferInfos,
                                 std::vector<VkDescriptorImageInfo>& imageInfos, bool dynamicUbo = 0);
        void updateDescriptorset(int startIndex, std::vector<VkDescriptorImageInfo>& imageInfos,
                                 bool isInputAttachment = 0);
    private:
    };
}


#endif
