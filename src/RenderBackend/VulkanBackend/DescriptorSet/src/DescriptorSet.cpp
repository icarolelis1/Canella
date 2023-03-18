#include "DescriptorSet/DescriptorSet.h"

using namespace Canella::RenderSystem::VulkanBackend;

void DescriptorSet::updateDescriptorset(int startIndex, std::vector<VkDescriptorImageInfo>& imageInfos,
                                        bool isInputAttachment)
{
}

void DescriptorSet::updateDescriptorset(std::vector<VkDescriptorBufferInfo>& bufferInfos,
                                        std::vector<VkDescriptorImageInfo>& imageInfos, bool dynamicUbo)
{
    std::vector<VkWriteDescriptorSet> writes;
    uint32_t i = 0;

    //write buffers, then images.
    for (auto& bufferInfo : bufferInfos)
    {
        //Write only one descriptor per time;
        VkWriteDescriptorSet w{};
        w.descriptorCount = 1;
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        if (dynamicUbo)
            w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        w.dstBinding = i;
        i++;
        w.pBufferInfo = &bufferInfo;
        w.dstSet = vk_descriptorset;
        writes.push_back(w);
    }

    for (auto& imageInfo : imageInfos)
    {
        //Write only one descriptor per time;
        VkWriteDescriptorSet w{};
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w.descriptorCount = 1;
        w.dstBinding = i;
        i++;
        w.pImageInfo = &imageInfo;
        w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        w.dstSet = vk_descriptorset;

        writes.push_back(w);
    }

    vkUpdateDescriptorSets(device->getLogicalDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0,NULL);
}
