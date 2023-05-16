#include "Resources/Resources.h"
#include "DescriptorSet/DescriptorSet.h"


Canella::RenderSystem::VulkanBackend::GPUResource::GPUResource(
        Canella::RenderSystem::VulkanBackend::ResourceType _type):type(_type) {

}

/**
 * \brief Wraps a Vulkan VkBuffer
 * \param device Vulkan device
 * \param size Size in bytes
 * \param usage Vulkan usage usage flags
 * \param properties Memory properties
 */
Canella::RenderSystem::VulkanBackend::Buffer::Buffer(Device *device,
                                                     VkDeviceSize size,
                                                     VkBufferUsageFlags usage,
                                                     VkMemoryPropertyFlags properties)
                                                     :GPUResource(ResourceType::BufferResource) {
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
    alloc_info.memoryTypeIndex = find_memory_type(device, mem_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device->getLogicalDevice(), &alloc_info, nullptr, &vk_deviceMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate buffer memory!");
    vkBindBufferMemory(device->getLogicalDevice(), vk_buffer, vk_deviceMemory, 0);
}

VkBuffer& Canella::RenderSystem::VulkanBackend::Buffer::getBufferHandle()
{
    return vk_buffer;
}

VkDeviceMemory& Canella::RenderSystem::VulkanBackend::Buffer::getMemoryHandle()
{
    return vk_deviceMemory;
}

Canella::RenderSystem::VulkanBackend::Buffer::~Buffer()
{
    vkDestroyBuffer(device->getLogicalDevice(), vk_buffer, device->getAllocator());
    vkFreeMemory(device->getLogicalDevice(), vk_deviceMemory, device->getAllocator());
}

uint32_t Canella::RenderSystem::VulkanBackend::Buffer::find_memory_type(Device* device, uint32_t typeFilter,
                                                                      VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(device->getPhysicalDevice(), &memory_properties);
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    return 0;
}

void Canella::RenderSystem::VulkanBackend::copy_buffer_to(
    VkCommandBuffer command_buffer,
    Buffer& src,
    Buffer& dst,
    VkDeviceSize device_size,
    VkQueue queue)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &beginInfo);
    VkBufferCopy copyRegion = {};
    copyRegion.size = device_size;
    vkCmdCopyBuffer(command_buffer, src.getBufferHandle(), dst.getBufferHandle(), 1, &copyRegion);
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    vkEndCommandBuffer(command_buffer);
    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
}

Canella::RenderSystem::VulkanBackend::ResourcesManager::~ResourcesManager() {
    auto it = resource_cache.begin();
    for(auto it = resource_cache.begin(); it != resource_cache.end();++it)
        it->second.reset();
}

Canella::RenderSystem::VulkanBackend::ResourcesManager::ResourcesManager(Device * _device) : device(_device) {}

Canella::RenderSystem::VulkanBackend::ResourceAccessor
Canella::RenderSystem::VulkanBackend::ResourcesManager::create_buffer(VkDeviceSize size,
                                                                      VkBufferUsageFlags usage,
                                                                      VkMemoryPropertyFlags properties) {
   auto unique_resource_id = uuid();
   resource_cache[unique_resource_id] = std::make_shared<Buffer>(device,size,usage,properties);

   return unique_resource_id;
}

Canella::RenderSystem::VulkanBackend::RefBuffer
Canella::RenderSystem::VulkanBackend::ResourcesManager::get_buffer_cached(uint64_t uuid) {
    auto cache_iterator = resource_cache.find(uuid);
    assert(cache_iterator != resource_cache.end());
    auto ref_buffer  = cache_iterator->second;
    assert(ref_buffer->type == ResourceType::BufferResource);

    return std::static_pointer_cast<Buffer>(ref_buffer);
}

uint64_t Canella::RenderSystem::VulkanBackend::ResourcesManager::write_descriptor_sets(
        VkDescriptorSet& descriptorset,
        std::vector<VkDescriptorBufferInfo> &buffer_infos,
        std::vector<VkDescriptorImageInfo> &image_infos)
{
    auto unique_id = uuid();

    DescriptorSet::update_descriptorset( device,
                                         descriptorset,
                                         buffer_infos,
                                         image_infos);

    return unique_id;
}


