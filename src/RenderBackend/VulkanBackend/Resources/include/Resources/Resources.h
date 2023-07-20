#ifndef VK_RESOURCES
#define VK_RESOURCES
#include <meshoptimizer.h>
#include "Device/Device.h"
#include <unordered_map>
#include "Commandpool/Commandpool.h"
#include "AsynchronousLoader/AsynchronousLoader.h"
#include "Render/Render.h"
#include <mutex>
#ifndef GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#endif

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {

            VkBufferMemoryBarrier bufferBarrier(VkBuffer buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
            VkImageMemoryBarrier imageBarrier(VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask);
            VkSampler create_sampler(VkDevice device, VkSamplerReductionModeEXT reductionMode);

            enum ResourceType
            {
                BufferResource,
                ImageResource
            };

           using ResourceAccessor = uint64_t;

            class GPUResource
            {
            public:
                std::string debug_id = "";
                VkDeviceSize size;
                explicit GPUResource(ResourceType type);
                virtual ~GPUResource() = default;
                ResourceType type;
                Event<> on_before_release;
            };

            class Buffer : public GPUResource
            {
            public:
                /**
                * \brief Wraps a Vulkan VkBuffer
                * \param device Vulkan device
                * \param size Size in bytes
                * \param usage Vulkan usage usage flags
                * \param properties Memory properties
                */
                Buffer(Device *_device, VkDeviceSize size,
                       VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
                ~Buffer() override;
                template <typename T>
                void udpate(T &data)
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
                void *mapPointer;
                Device *device;

            private:
                bool mapped = false;
                VkBuffer vk_buffer;
            };
            using RefGPUResource = std::shared_ptr<GPUResource>;
            using RefBuffer = std::shared_ptr<Buffer>;

            uint32_t find_memory_type(Device *device, uint32_t typeFilter, VkMemoryPropertyFlags properties);

            class Image : public GPUResource
            {
            private:
                int num_layers;
                Device *device;
                VkExtent2D extent;

            public:
                Image(Device *_device,
                      uint32_t Width, uint32_t Height,
                      VkFormat format,
                      VkImageTiling tiling,
                      VkImageUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      uint32_t num_mips,
                      VkImageAspectFlags aspectFlags,
                      uint32_t arrayLayers = 1,
                      bool useMaxNumMips = false,
                      VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

                ~Image() override;
                VkImage image;
                VkDeviceMemory memory;
                VkImageView view;
            };
            using RefImage = std::shared_ptr<Image>;

            /**
             * Resource Manager cache all the resoruces used by the application (Buffers and Images)
             */
            class ResourcesManager
            {

            public:
                RefBuffer get_buffer_cached(uint64_t);
                RefImage get_image_cached(uint64_t);

                explicit ResourcesManager(Device *device);
                AsynchronousLoader async_loader;
                /**
                 * @brief late setup. Builds the AsyncronousLoader
                 */
                void build();

                /**
                 * @brief creates a vkbuffer and return an unique id to access the buffer using get_buffer_cached(id)
                 * @param size size of the buffer
                 * @param usage usage flags for the buffer
                 * @param properties memories properties used by the buffer
                 */
                ResourceAccessor create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

                /**
                 * @brief Wrapper for Image Resource
                 * params...
                 */
                ResourceAccessor create_image(Device *device,
                                              uint32_t width,
                                              uint32_t height,
                                              VkFormat format,
                                              VkImageTiling tilling,
                                              VkImageUsageFlags usage,
                                              VkMemoryPropertyFlags properties,
                                              uint32_t num_mips,
                                              VkImageAspectFlags aspectFlags,
                                              uint32_t arrayLayers = 1,
                                              bool useMaxNumMips = false,
                                              VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

                ~ResourcesManager() = default;
                template <typename Data>
                /**
                 * @brief Creates a Device Local Buffer
                 * @tparam Data
                 * @param size
                 * @param flags
                 * @param transfer_pool
                 * @param data
                 * @return
                 */
                uint64_t create_storage_buffer(size_t size, VkBufferUsageFlags flags, VulkanBackend::Commandpool *transfer_pool, Data *data)
                {

                    auto staging_buffer = std::make_shared<Buffer>(device,
                                                                   size,
                                                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
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

                void copy_buffer_to(VkCommandBuffer command_buffer, const RefBuffer &src, const RefBuffer &dst, size_t device_size, VkQueue queue);
                uint64_t write_descriptor_sets(VkDescriptorSet &descriptorset, std::vector<VkDescriptorBufferInfo> &buffer_infos, std::vector<VkDescriptorImageInfo> &image_infos, bool);
                void destroy_resources();

                // Events
                Event<VkSemaphore&> OnTransferCommand;

            private:
                std::unordered_map<ResourceAccessor, RefGPUResource> resource_cache;
                Device *device;
                std::mutex mutex;
            };
        }
    } // namespace name

}

#endif