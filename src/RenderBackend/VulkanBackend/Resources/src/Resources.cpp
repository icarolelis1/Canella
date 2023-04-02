#include "Resources/Resources.h"

void Canella::RenderSystem::VulkanBackend::Buffer::allocateBuffer(Device *device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device->getLogicalDevice(), &bufferInfo, nullptr, &vk_buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device->getLogicalDevice(), vk_buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(device, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device->getLogicalDevice(), &allocInfo, nullptr, &vk_deviceMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device->getLogicalDevice(), vk_buffer, vk_deviceMemory, 0);
}
VkBuffer &Canella::RenderSystem::VulkanBackend::Buffer::getBufferHandle()
{
    return vk_buffer;
}

VkDeviceMemory &Canella::RenderSystem::VulkanBackend::Buffer::getMemoryHandle()
{
    return vk_deviceMemory;
}
void Canella::RenderSystem::VulkanBackend::Buffer::destroy(Device &device)
{
    vkDestroyBuffer(device.getLogicalDevice(), vk_buffer, device.getAllocator());
    vkFreeMemory(device.getLogicalDevice(), vk_deviceMemory, device.getAllocator());
}

uint32_t Canella::RenderSystem::VulkanBackend::Buffer::findMemoryType(Device *device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(device->getPhysicalDevice(), &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    return 0;
}