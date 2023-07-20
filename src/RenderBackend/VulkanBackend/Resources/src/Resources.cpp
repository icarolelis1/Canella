#include "Resources/Resources.h"
#include "DescriptorSet/DescriptorSet.h"
#include "CanellaUtility/CanellaUtility.h"

using namespace Canella::RenderSystem::VulkanBackend;

//Utility

VkSampler Canella::RenderSystem::VulkanBackend::create_sampler(VkDevice device, VkSamplerReductionModeEXT reductionMode)
{
    VkSamplerCreateInfo createInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

    createInfo.magFilter = VK_FILTER_LINEAR;
    createInfo.minFilter = VK_FILTER_LINEAR;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    createInfo.minLod = 0;
    createInfo.maxLod = 16.f;

    VkSamplerReductionModeCreateInfoEXT createInfoReduction = { VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT };

    if (reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE_EXT)
    {
        createInfoReduction.reductionMode = reductionMode;

        createInfo.pNext = &createInfoReduction;
    }

    VkSampler sampler = 0;
    VK_CHECK(vkCreateSampler(device, &createInfo, 0, &sampler),"Failed to Create Sampler");
    return sampler;
}

VkBufferMemoryBarrier Canella::RenderSystem::VulkanBackend::bufferBarrier(VkBuffer buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
    VkBufferMemoryBarrier result = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};

    result.srcAccessMask = srcAccessMask;
    result.dstAccessMask = dstAccessMask;
    result.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    result.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    result.buffer = buffer;
    result.offset = 0;
    result.size = VK_WHOLE_SIZE;
    return result;
}

VkImageMemoryBarrier Canella::RenderSystem::VulkanBackend::imageBarrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask)
{
    VkImageMemoryBarrier result = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };

    result.srcAccessMask = srcAccessMask;
    result.dstAccessMask = dstAccessMask;
    result.oldLayout = oldLayout;
    result.newLayout = newLayout;
    result.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    result.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    result.image = image;
    result.subresourceRange.aspectMask = aspectMask;
    result.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    result.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    return result;
}



uint32_t Canella::RenderSystem::VulkanBackend::find_memory_type(Device *device,
                                                                uint32_t typeFilter,
                                                                VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(device->getPhysicalDevice(), &memory_properties);
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    assert(!"No compatible memory type found");
    return ~0u;
}

GPUResource::GPUResource(
    ResourceType _type) : type(_type)
{
}

Buffer::Buffer(RenderSystem::VulkanBackend::Device *device,
               VkDeviceSize size,
               VkBufferUsageFlags usage,
               VkMemoryPropertyFlags properties)
    : GPUResource(ResourceType::BufferResource)
{
    this->device = device;
    this->size = size;
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // todo make buffer manages his own bufferInfoDescriptor
    if (vkCreateBuffer(device->getLogicalDevice(), &bufferInfo, nullptr, &vk_buffer) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer!");

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(device->getLogicalDevice(), vk_buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(device, mem_requirements.memoryTypeBits, properties);

    VkMemoryAllocateFlagsInfo flagInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        alloc_info.pNext = &flagInfo;
        flagInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        flagInfo.deviceMask = 1;
    }
    auto allocate_result = vkAllocateMemory(device->getLogicalDevice(),
                                            &alloc_info,
                                            nullptr,
                                            &vk_deviceMemory);
    if (allocate_result != VK_SUCCESS)
        throw std::runtime_error("failed to allocate buffer memory!");
    if (vkBindBufferMemory(device->getLogicalDevice(), vk_buffer, vk_deviceMemory, 0) != VK_SUCCESS)
        throw std::runtime_error("failed to bind buffer memory!");
}

VkBuffer &Buffer::getBufferHandle()
{
    return vk_buffer;
}

VkDeviceMemory &Buffer::getMemoryHandle()
{
    return vk_deviceMemory;
}

Buffer::~Buffer()
{
    vkDestroyBuffer(device->getLogicalDevice(), vk_buffer, device->getAllocator());
    vkFreeMemory(device->getLogicalDevice(), vk_deviceMemory, device->getAllocator());
}

void Buffer::unmap()
{
    if (mapped)
        vkUnmapMemory(device->getLogicalDevice(), vk_deviceMemory);
}

void Buffer::flush(VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = vk_deviceMemory;
    mappedRange.offset = offset;
    mappedRange.size = VK_WHOLE_SIZE;
    if (vkFlushMappedMemoryRanges(device->getLogicalDevice(), 1, &mappedRange) != VK_SUCCESS)
        Canella::Logger::Error("Failed to flush memory for buffer %s", &debug_id);
}

void ResourcesManager::copy_buffer_to(VkCommandBuffer command_buffer, const RefBuffer &src, const RefBuffer &dst, VkDeviceSize device_size, VkQueue queue)
{ 
    // vkWaitForFences(device->getLogicalDevice(), 1, &async_loader.fence, VK_TRUE, UINT64_MAX);
    // vkResetFences(device->getLogicalDevice(), 1, &async_loader.fence);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.size = device_size;

    vkCmdCopyBuffer(command_buffer,
                    src->getBufferHandle(),
                    dst->getBufferHandle(),
                    1,
                    &copyRegion);
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    // submit_info.pSignalSemaphores = &async_loader.semaphore;
    // submit_info.signalSemaphoreCount = 1;
    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    //OnTransferCommand.invoke(async_loader.semaphore);
    vkQueueWaitIdle(queue);
}

ResourcesManager::ResourcesManager(Device *_device) : device(_device), async_loader(_device) {}

// Build the async_loader creating the synchronization obje
void ResourcesManager::build() { async_loader.build(); }

ResourceAccessor ResourcesManager::create_buffer(size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    mutex.lock();
    auto unique_resource_id = uuid();
    resource_cache[unique_resource_id] = std::make_shared<Buffer>(device, size, usage, properties);
    resource_cache[unique_resource_id]->debug_id = "Cant destroy";
    mutex.unlock();
    return unique_resource_id;
}

ResourceAccessor ResourcesManager::create_image(
    Device *device,
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tilling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImageCreateFlags flags,
    VkImageAspectFlags aspectFlags,
    uint32_t arrayLayers,
    bool useMaxNumMips,
    VkSampleCountFlagBits samples)
{
    auto unique_resource_id = uuid();
    resource_cache[unique_resource_id] = std::make_shared<Image>(device, width, height, format, tilling, usage, properties,
                                                                 flags, aspectFlags, arrayLayers, useMaxNumMips, samples);

    return unique_resource_id;
}

RefBuffer ResourcesManager::get_buffer_cached(uint64_t uuid)
{
    auto cache_iterator = resource_cache.find(uuid);
    assert(cache_iterator != resource_cache.end());
    auto ref_buffer = cache_iterator->second;
    assert(ref_buffer->type == ResourceType::BufferResource);
    return std::static_pointer_cast<Buffer>(ref_buffer);
}

RefImage
ResourcesManager::get_image_cached(uint64_t uuid)
{
    auto cache_iterator = resource_cache.find(uuid);
    assert(cache_iterator != resource_cache.end());
    auto ref_image = cache_iterator->second;
    assert(ref_image->type == ResourceType::ImageResource);
    return std::static_pointer_cast<Image>(ref_image);
}

uint64_t ResourcesManager::write_descriptor_sets(
    VkDescriptorSet &descriptorset,
    std::vector<VkDescriptorBufferInfo> &buffer_infos,
    std::vector<VkDescriptorImageInfo> &image_infos,
    bool storage_buffers)
{
    auto unique_id = uuid();
    DescriptorSet::update_descriptorset(device, descriptorset, buffer_infos, image_infos, false, storage_buffers);
    return unique_id;
}

void ResourcesManager::destroy_resources()
{
    auto it = resource_cache.begin();
    for (auto it = resource_cache.begin(); it != resource_cache.end(); ++it)
        it->second.reset();
}

Image::Image(Device *_device,
             uint32_t Width,
             uint32_t Height,
             VkFormat format,
             VkImageTiling tiling,
             VkImageUsageFlags usage,
             VkMemoryPropertyFlags properties,
             VkImageCreateFlags flags,
             VkImageAspectFlags aspectFlags,
             uint32_t arrayLayers,
             bool useMaxNumMips,
             VkSampleCountFlagBits samples) : GPUResource(ResourceType::ImageResource)
{
    uint32_t numMips = 1;
    auto get_image_mips_count = [=](uint32_t w,uint32_t h)
    {
        auto count = 0;
        while(h > 1 || w > 1)
        {
            count++;
            h /= 2;
            w /=2;
        }
        return count;
    };

    if (useMaxNumMips)
    {
        numMips = get_image_mips_count(Width,Height);
    }
    extent.width = Width;
    extent.height = Height;
    device = _device;
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = Width;
    imageInfo.extent.height = Height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = numMips;
    imageInfo.arrayLayers = arrayLayers;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = samples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device->getLogicalDevice(), &imageInfo, device->getAllocator(), &image) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create image\n");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device->getLogicalDevice(), image, &memRequirements);
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(device, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device->getLogicalDevice(), &allocInfo, nullptr, &memory) != VK_SUCCESS)
    {
        throw std::runtime_error("    Failed to allocate image memory!\n");
    }

    vkBindImageMemory(device->getLogicalDevice(), image, memory, 0);

    // Image View Creation
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = numMips;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    VkResult result;
    result = vkCreateImageView(device->getLogicalDevice(), &viewInfo, nullptr, &view);

    VK_CHECK(result, "Faild to create imageview");
}

Image::~Image()
{

    on_before_release.invoke();
    vkDestroyImageView(device->getLogicalDevice(), view, device->getAllocator());
    vkDestroyImage(device->getLogicalDevice(), image, device->getAllocator());
    vkFreeMemory(device->getLogicalDevice(), memory, device->getAllocator());
}
