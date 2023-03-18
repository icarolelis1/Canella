#pragma once
#ifndef DESCRIPTOR_SET
#define DESCRIPTOR_SET
#include <VulkanRender/VulkanRender.h>

namespace Canella::RenderSystem::VulkanBackend
{
    class DescriptorSet
    {
    public:
        DescriptorSet(const Device* _device);
        DescriptorSet(const DescriptorSet& other);

        void updateDescriptorset(std::vector<VkDescriptorBufferInfo>& bufferInfos,
                                 std::vector<VkDescriptorImageInfo>& imageInfos, bool dynamicUbo = 0);
        void updateDescriptorset(int startIndex, std::vector<VkDescriptorImageInfo>& imageInfos,
                                 bool isInputAttachment = 0);


        VkDescriptorSet& getDescriptorSetHandle();

    private:
        Device* device;


        VkDescriptorSet vk_descriptorset;
    };
}


#endif
