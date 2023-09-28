#include "DescriptorSet/DescriptorSet.h"

using namespace Canella::RenderSystem::VulkanBackend;


void DescriptorSet::update_descriptorset(Device *device, VkDescriptorSet &descriptorset,
                                         std::vector<VkDescriptorBufferInfo> &bufferInfos,
                                         std::vector<VkDescriptorImageInfo> &imageInfos, bool dynamicUbo, bool storage)
{
    std::vector<VkWriteDescriptorSet> writes;
    uint32_t i = 0;
    for (auto& buffer_info : bufferInfos)
    {
        VkWriteDescriptorSet w{};
        w.descriptorCount = 1;
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        if(storage)
            w.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        else 
            w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        w.dstBinding = i;
        i++;
        w.pBufferInfo = &buffer_info;
        w.dstSet = descriptorset;
        writes.push_back(w);
    }
    for (auto& image_info : imageInfos)
    {
        VkWriteDescriptorSet w{};
        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w.descriptorCount = 1;
        w.dstBinding = i;
        i++;
        w.pImageInfo = &image_info;
        w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        w.dstSet = descriptorset;
        writes.push_back(w);
    }

    vkUpdateDescriptorSets(device->getLogicalDevice(),
                           static_cast<uint32_t>(writes.size()),
                           writes.data(),
                           0,
                           nullptr);
}

