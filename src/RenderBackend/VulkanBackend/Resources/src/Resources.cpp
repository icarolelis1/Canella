#include "Resources/Resources.h"
#include "DescriptorSet/DescriptorSet.h"
#include "CanellaUtility/CanellaUtility.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "ktx.h"
#include "ktxvulkan.h"

using namespace Canella::RenderSystem::VulkanBackend;

VkSampler Canella::RenderSystem::VulkanBackend::create_sampler(VkDevice device, VkSamplerReductionModeEXT reductionMode)
{
    VkSamplerCreateInfo create_info = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

    create_info.magFilter    = VK_FILTER_LINEAR;
    create_info.minFilter    = VK_FILTER_LINEAR;
    create_info.mipmapMode   = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    create_info.minLod       = 0;
    create_info.maxLod       = 16.f;
    create_info.anisotropyEnable = VK_TRUE;
    create_info.maxAnisotropy = 10;
    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    create_info.unnormalizedCoordinates = VK_FALSE;
    VkSamplerReductionModeCreateInfoEXT createInfoReduction = { VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT };

    if (reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE_EXT)
    {
        createInfoReduction.reductionMode = reductionMode;
        create_info.pNext                 = &createInfoReduction;
    }

    VkSampler sampler ;
    VK_CHECK( vkCreateSampler( device, &create_info, 0, &sampler), "Failed to Create Sampler");
    return sampler;
}

VkBufferMemoryBarrier Canella::RenderSystem::VulkanBackend::buffer_barrier( VkBuffer buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
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

VkImageMemoryBarrier Canella::RenderSystem::VulkanBackend::image_barrier( VkImage image,
                                                                          VkAccessFlags srcAccessMask,
                                                                          VkAccessFlags dstAccessMask,
                                                                          VkImageLayout oldLayout,
                                                                          VkImageLayout newLayout,
                                                                          VkImageAspectFlags aspectMask)
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
     vkWaitForFences(device->getLogicalDevice(), 1, &async_loader.fence, VK_TRUE, UINT64_MAX);
     vkResetFences(device->getLogicalDevice(), 1, &async_loader.fence);

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
    vkQueueSubmit(queue, 1, &submit_info, async_loader.fence);
    //OnTransferCommand.invoke(async_loader.semaphore);
    vkQueueWaitIdle(queue);
}

ResourcesManager::ResourcesManager(Device *_device) : device(_device), async_loader(_device), async_loader2(_device) {}

// Build the async_loader creating the synchronization obje
void ResourcesManager::build() {
    resource_loader_pool.build(device,POOL_TYPE::GRAPHICS,VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
    async_loader.build();
    async_loader2.build();
    vkResetFences(device->getLogicalDevice(), 1, &async_loader2.fence);

}

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
        uint32_t num_mips,
        VkImageAspectFlags aspectFlags,
        uint32_t arrayLayers,
        VkSampleCountFlagBits samples,
        bool store_in_textures_cache)
{
    auto unique_resource_id = uuid();
    if(!store_in_textures_cache)
        resource_cache[unique_resource_id] = std::make_shared<Image>(device,width,height,format,tilling,usage,properties,num_mips,aspectFlags,arrayLayers);
    else
        textures_cache[unique_resource_id] = std::make_shared<Image>(device,width,height,format,tilling,usage,properties,num_mips,aspectFlags,arrayLayers);
    return unique_resource_id;
}


ResourceAccessor ResourcesManager::create_texture( const std::string &file_path, Device *device, VkFormat format )
{
    int      tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load(file_path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    const uint32_t maxMips = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;

    if (!pixels) {
        Canella::Logger::Error("Failed to load texture at source path %s",file_path.c_str());
        return 0;
    }

    VkDeviceSize image_size = tex_width * tex_height * 4;
    {
        std::unique_lock<std::mutex> m( async_loader.pool_mutex );
        auto resource_accessor = create_image( device, tex_width, tex_height, format, VK_IMAGE_TILING_OPTIMAL,
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                      0,
                                      maxMips,
                                      VK_IMAGE_ASPECT_COLOR_BIT,
                                      1,
                                      VK_SAMPLE_COUNT_1_BIT,
                                      true);


        auto  staging_buffer = std::make_shared<Buffer>( device, image_size,
                                                                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );
        void *data;
        vkMapMemory( device->getLogicalDevice(), staging_buffer->vk_deviceMemory, 0, image_size, 0, &data );
        memcpy( data, pixels, static_cast<size_t>(image_size));
        vkUnmapMemory( device->getLogicalDevice(), staging_buffer->vk_deviceMemory );

        auto image = std::static_pointer_cast<Image>(textures_cache[resource_accessor]);
        auto command_buffer = resource_loader_pool.requestCommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        resource_loader_pool.begin_command_buffer(device,command_buffer,true);

        auto layout_barrier = image_barrier( image->image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT );

        vkCmdPipelineBarrier(command_buffer,VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,VK_DEPENDENCY_BY_REGION_BIT,0,
            0, 0,0,1,
            &layout_barrier);

        copy_buffer_to_image( tex_width, tex_height, staging_buffer, image, command_buffer );

      /*  auto layout_to_shader_readonly = image_barrier( image->image, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT );
        vkCmdPipelineBarrier(command_buffer,VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,VK_DEPENDENCY_BY_REGION_BIT,0,
                             0, 0,0,1,
                             &layout_to_shader_readonly);*/


        generate_mips(image,command_buffer,tex_width,tex_height,maxMips);

        resource_loader_pool.endCommandBuffer(command_buffer);
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &command_buffer;

        {
            std::unique_lock<std::mutex> lock(device->get_queue_mutex(1));
            vkQueueSubmit(device->getGraphicsQueueHandle(), 1, &submitInfo,  async_loader2.fence);
        }

        vkWaitForFences(device->getLogicalDevice(), 1, &async_loader2.fence, VK_TRUE, UINT64_MAX);
        vkResetFences(device->getLogicalDevice(), 1, &async_loader2.fence);
        return resource_accessor;
    }
}


ResourceAccessor ResourcesManager::create_cube_map( const std::string& path  )
{
    ktxTexture *ktxTexture;
    ktxResult   result   = ktxTexture_CreateFromNamedFile(path.c_str(),KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture );

    if ( result != KTX_SUCCESS ) {
        Canella::Logger::Error( "Failed to load ktx texture %s", path.c_str());
        return 0;
    }

    auto width      = ktxTexture->baseWidth;
    auto height     = ktxTexture->baseHeight;
    auto mip_levels = ktxTexture->numLevels;
    VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
    ktx_uint8_t *ktxTextureData = ktxTexture_GetData( ktxTexture );
    ktx_size_t  ktxTextureSize  = ktxTexture_GetDataSize(ktxTexture);
    {
        std::unique_lock<std::mutex>m( async_loader.pool_mutex );

        auto staging_buffer = std::make_shared<Buffer>( device, ktxTextureSize,
                                                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

        void *data;
        vkMapMemory( device->getLogicalDevice(), staging_buffer->vk_deviceMemory, 0, ktxTextureSize, 0, &data );
        memcpy( data, ktxTextureData, static_cast<size_t>(ktxTextureSize));
        vkUnmapMemory( device->getLogicalDevice(), staging_buffer->vk_deviceMemory );

        std::vector<VkBufferImageCopy> copy_regions;

        for (uint32_t face = 0; face < 6; face++)
        {
            for (uint32_t level = 0; level < mip_levels; level++)
            {
                ktx_size_t offset;
                KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, level, 0, face, &offset);
                assert(result == KTX_SUCCESS);

                VkBufferImageCopy buffer_image_copy = {};
                buffer_image_copy.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
                buffer_image_copy.imageSubresource.mipLevel       = level;
                buffer_image_copy.imageSubresource.baseArrayLayer = face;
                buffer_image_copy.imageSubresource.layerCount     = 1;
                buffer_image_copy.imageExtent.width               = ktxTexture->baseWidth >> level;
                buffer_image_copy.imageExtent.height              = ktxTexture->baseHeight >> level;
                buffer_image_copy.imageExtent.depth               = 1;
                buffer_image_copy.bufferOffset                    = offset;

                copy_regions.push_back( buffer_image_copy);
            }
        }
        auto cube_map_accessor = create_image( device, width, height, format, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                               0,
                                               mip_levels,
                                               VK_IMAGE_ASPECT_COLOR_BIT,
                                               6,
                                               VK_SAMPLE_COUNT_1_BIT,
                                               true);


        auto command_buffer = resource_loader_pool.requestCommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        VkImageSubresourceRange subresource_range = {};
        subresource_range.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource_range.baseMipLevel = 0;
        subresource_range.levelCount   = mip_levels;
        subresource_range.layerCount   = 6;

        resource_loader_pool.begin_command_buffer(device,command_buffer,true);
        auto cube_map = get_texture_cached( cube_map_accessor);

        VkImageMemoryBarrier transfer_barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };

        transfer_barrier.srcAccessMask = 0;
        transfer_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        transfer_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        transfer_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        transfer_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        transfer_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        transfer_barrier.image = cube_map->image;
        transfer_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        transfer_barrier.subresourceRange.levelCount = mip_levels;
        transfer_barrier.subresourceRange.layerCount = 6;
        transfer_barrier.subresourceRange.baseMipLevel = 0;


        vkCmdPipelineBarrier(command_buffer,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT,
                             0,0,0,0,1,
                             &transfer_barrier);

        vkCmdCopyBufferToImage(command_buffer,
                               staging_buffer->getBufferHandle(),
                               cube_map->image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               static_cast<uint32_t>(copy_regions.size()),
                               copy_regions.data());


        VkImageMemoryBarrier shader_read_barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };

        shader_read_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        shader_read_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        shader_read_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        shader_read_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        shader_read_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        shader_read_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        shader_read_barrier.image = cube_map->image;
        shader_read_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        shader_read_barrier.subresourceRange.levelCount = mip_levels;
        shader_read_barrier.subresourceRange.layerCount = 6;
        shader_read_barrier.subresourceRange.baseMipLevel = 0;

        vkCmdPipelineBarrier(command_buffer,VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,VK_DEPENDENCY_BY_REGION_BIT,0,
                             0, 0,0,1,
                             &shader_read_barrier);

        resource_loader_pool.endCommandBuffer(command_buffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &command_buffer;
        {
            std::unique_lock<std::mutex> lock(device->get_queue_mutex(1));
            vkQueueSubmit(device->getGraphicsQueueHandle(), 1, &submitInfo,  async_loader2.fence);
        }
        vkWaitForFences(device->getLogicalDevice(), 1, &async_loader2.fence, VK_TRUE, UINT64_MAX);
        vkResetFences(device->getLogicalDevice(), 1, &async_loader2.fence);
        ktxTexture_Destroy( ktxTexture );

        return cube_map_accessor;
    }

}

void ResourcesManager::generate_mips(std::shared_ptr<Image> image,VkCommandBuffer command, uint32_t width,uint32_t height,uint32_t mip_levels)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image->image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mip_width  = width;
    int32_t mip_height = height;

    for (uint32_t i = 1; i < mip_levels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(command,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,0,0,
                             nullptr,0,
                             nullptr,1,&barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = { mip_width, mip_height, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(command,
                       image->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(command,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        if ( mip_width > 1) mip_width /= 2;
        if ( mip_height > 1) mip_height /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mip_levels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(command,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);
}

void ResourcesManager::copy_buffer_to_image( int tex_width, int tex_height, std::shared_ptr<Buffer> &staging_buffer,
                                             std::shared_ptr<Image> &image, VkCommandBuffer command_buffer ) const {
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };

    region.imageExtent = {static_cast<uint32_t>(tex_width),static_cast<uint32_t>(tex_height),1};

    vkCmdCopyBufferToImage(command_buffer, staging_buffer->getBufferHandle(), image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
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

RefImage
ResourcesManager::get_texture_cached( uint64_t uuid) {
    auto cache_iterator = textures_cache.find(uuid);
    assert(cache_iterator != textures_cache.end());
    auto ref_image = cache_iterator->second;
    assert(ref_image->type == ResourceType::ImageResource);
    return std::static_pointer_cast<Image>(ref_image);
}


uint64_t ResourcesManager::create_storage_buffer(size_t size, VkBufferUsageFlags flags, VulkanBackend::Commandpool *transfer_pool, void *data)
{
    auto staging_buffer = std::make_shared<Buffer>(device,size,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    staging_buffer->debug_id = "Staged";
    if (auto result = vkMapMemory(device->getLogicalDevice(), staging_buffer->vk_deviceMemory, 0, size, 0, &staging_buffer->mapPointer) != VK_SUCCESS)
        throw std::runtime_error("Failed to map buffer memory");

    memcpy(staging_buffer->mapPointer, data, size);
    staging_buffer->unmap();
    ResourceAccessor id = create_buffer(size, flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    const auto command = transfer_pool->requestCommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    copy_buffer_to(command, staging_buffer, get_buffer_cached(id), size, device->getTransferQueueHandle());

    return id;

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

void ResourcesManager::destroy_non_persistent_resources()
{
    auto it = resource_cache.begin();
    for (auto it = resource_cache.begin(); it != resource_cache.end(); ++it)
    {
        if(it->second->is_persistent == false)
            it->second.reset();
    }

    resource_cache.clear();
}

void ResourcesManager::destroy_texture_resources() {

    auto it = textures_cache.begin();
    for (auto it = textures_cache.begin(); it != textures_cache.end(); ++it)
    {
        if(it->second->is_persistent == true)
            it->second.reset();
    }

    textures_cache.clear();
}


Image::Image(Device *_device,
             uint32_t Width, uint32_t Height,
             VkFormat format,
             VkImageTiling tiling,
             VkImageUsageFlags usage,
             VkMemoryPropertyFlags properties,
             uint32_t num_mips,
             VkImageAspectFlags aspectFlags,
             uint32_t array_layers,
             VkSampleCountFlagBits samples ) : GPUResource(ResourceType::ImageResource)
{

    extent.width = Width;
    extent.height = Height;
    device = _device;
    VkImageCreateInfo image_info = {};
    image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType     = VK_IMAGE_TYPE_2D;
    image_info.extent.width  = Width;
    image_info.extent.height = Height;
    image_info.extent.depth  = 1;
    image_info.mipLevels     = num_mips;
    image_info.arrayLayers   = array_layers;
    image_info.format        = format;
    image_info.tiling        = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage         = usage;
    image_info.samples       = samples;
    image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    if( array_layers == 6)
    {
        Canella::Logger::Trace("IMAGE WITH SIX LAYERS");
        image_info.flags     = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }



    if ( vkCreateImage( device->getLogicalDevice(), &image_info, device->getAllocator(), &image) != VK_SUCCESS)
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
    viewInfo.viewType = array_layers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = num_mips;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = array_layers;
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
