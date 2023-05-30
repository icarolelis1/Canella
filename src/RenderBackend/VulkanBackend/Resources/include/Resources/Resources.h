#ifndef VK_RESOURCES
#define VK_RESOURCES
#include <meshoptimizer.h>
#include "Device/Device.h"
#include <unordered_map>
#include "Commandpool/Commandpool.h"
#include "Render/Render.h"
#ifndef GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#endif


namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            enum ResourceType{
                BufferResource,
                ImageResource
            };

            using ResourceAccessor = uint64_t;

            class GPUResource{
            public:
                std::string debug_id= "";
                VkDeviceSize size;
                explicit GPUResource(ResourceType type);
                virtual ~GPUResource() = default;
                ResourceType type;

            };

            class Buffer : public GPUResource
            {
            public:
                Buffer(Device *_device, VkDeviceSize size,
                       VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
                ~Buffer() override;
                template<typename T>
                void udpate(T& data)
                {
                    // Keep persistent mapped buffers
                    if (!mapped)
                        vkMapMemory(device->getLogicalDevice(),
                                    vk_deviceMemory,
                                    0,
                                    size,
                                    0,
                                    &mapPointer);

                    memcpy(mapPointer, &data, size);
                    mapped = true;
                }
                void unmap();
                void flush(VkDeviceSize offset);
                VkBuffer &getBufferHandle();
                VkDeviceMemory &getMemoryHandle();

                VkDeviceMemory vk_deviceMemory;
                void* mapPointer;
                Device* device;
            private:
                bool mapped = false;
                VkBuffer vk_buffer;

            };
            using RefGPUResource = std::shared_ptr<GPUResource>;
            using RefBuffer = std::shared_ptr<Buffer>;
            void copy_buffer_to(            VkCommandBuffer command_buffer,
                                            const RefBuffer& src,
                                            const RefBuffer& dst,
                                            size_t device_size,
                                            VkQueue queue);
            uint32_t find_memory_type(Device *device, uint32_t typeFilter, VkMemoryPropertyFlags properties);


            class Image : public GPUResource{
            private:
                int num_layers;
                Device* device;
                VkExtent2D extent;
            public:
                Image(Device* _device,
                      uint32_t Width, uint32_t Height,
                      VkFormat format,
                      VkImageTiling tiling,
                      VkImageUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkImageCreateFlags flags, VkImageAspectFlags aspectFlags,
                      uint32_t arrayLayers =1 ,
                      bool useMaxNumMips = true,
                      VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
                ~Image() override;
                VkImage image;
                VkDeviceMemory  memory;
                VkImageView view;
            };
            using RefImage = std::shared_ptr<Image>;


            class ResourcesManager{
            private:
                std::unordered_map<ResourceAccessor,RefGPUResource> resource_cache;
                Device* device;

            public:
                RefBuffer get_buffer_cached(uint64_t);
                RefImage get_image_cached(uint64_t);

                explicit ResourcesManager(Device* device);
                ResourceAccessor create_buffer(VkDeviceSize size,
                                               VkBufferUsageFlags usage,
                                               VkMemoryPropertyFlags properties);
                ResourceAccessor  create_image(Device* device,
                                               uint32_t width,
                                               uint32_t height,
                                               VkFormat format,
                                               VkImageTiling tilling,
                                               VkImageUsageFlags usage,
                                               VkMemoryPropertyFlags properties,
                                               VkImageCreateFlags flags, VkImageAspectFlags aspectFlags,
                                               uint32_t arrayLayers =1 ,
                                               bool useMaxNumMips = true,
                                               VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

                ~ResourcesManager() = default;
                void destroy_resources();
                template<typename Data>
                uint64_t  create_storage_buffer(size_t size,
                                                VkBufferUsageFlags flags,
                                                VulkanBackend::Commandpool* transfer_pool,
                                                Data* data)
                {

                    auto staging_buffer = std::make_shared<Buffer>(device,
                                                                    size,
                                                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
                    staging_buffer->debug_id = "Staged";

                    if(vkMapMemory(device->getLogicalDevice(),
                                   staging_buffer->vk_deviceMemory,
                                   0,
                                   size,
                                   0,
                                   &staging_buffer->mapPointer)!=VK_SUCCESS)
                        throw std::runtime_error("Failed to map buffer memory");


                    memcpy(staging_buffer->mapPointer, data, size);
                    staging_buffer->unmap();

                    ResourceAccessor id = create_buffer(size,
                                                        flags,
                                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                    const auto command = transfer_pool->requestCommandBuffer(
                            device,
                            VK_COMMAND_BUFFER_LEVEL_PRIMARY );

                    copy_buffer_to(command,
                                   staging_buffer,
                                   get_buffer_cached(id),
                                   size,
                                   device->getTransferQueueHandle());

                    return id;
                }

                uint64_t
                write_descriptor_sets(VkDescriptorSet& descriptorset,
                                      std::vector<VkDescriptorBufferInfo> &buffer_infos,
                                      std::vector<VkDescriptorImageInfo> &image_infos,bool);
            };
        }
    } // namespace name

}

#endif