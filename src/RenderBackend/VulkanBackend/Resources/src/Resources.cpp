#include "Resources/Resources.h"

/**
 * \brief Wraps a Vulkan VkBuffer
 * \param device Vulkan device
 * \param size Size in bytes
 * \param usage Vulkan usage usage flags
 * \param properties Memory properties
 */
Canella::RenderSystem::VulkanBackend::Buffer::Buffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage,
                                                     VkMemoryPropertyFlags properties)
{
    this->device = device;
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device->getLogicalDevice(), &bufferInfo, nullptr, &vk_buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer!");
    
    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device->getLogicalDevice(), vk_buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = findMemoryType(device, mem_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device->getLogicalDevice(), &alloc_info, nullptr, &vk_deviceMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate buffer memory!");
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
void Canella::RenderSystem::VulkanBackend::Buffer::destroy(Device &device) const
{
    vkDestroyBuffer(device.getLogicalDevice(), vk_buffer, device.getAllocator());
    vkFreeMemory(device.getLogicalDevice(), vk_deviceMemory, device.getAllocator());
}

uint32_t Canella::RenderSystem::VulkanBackend::Buffer::findMemoryType(Device *device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(device->getPhysicalDevice(), &memory_properties);
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    return 0;
}