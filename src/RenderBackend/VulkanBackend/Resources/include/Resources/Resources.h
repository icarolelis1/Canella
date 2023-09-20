#ifndef VK_RESOURCES
#define VK_RESOURCES
#include <meshoptimizer.h>
#include "Device/Device.h"
#include <unordered_map>
#include "Commandpool/Commandpool.h"
#include "AsynchronousLoader/SyncStructure.h"
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

            VkBufferMemoryBarrier buffer_barrier( VkBuffer buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask);
            VkImageMemoryBarrier image_barrier( VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask);
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
                bool is_persistent = false;
                VkDeviceSize size;
                explicit GPUResource(ResourceType type);
                virtual ~GPUResource() = default;
                ResourceType type;
                Event<> on_before_release;
            };

            //Todo refactor this class. No need to use inheritance here just use separate
            // arrays to store each type of resource. No need to use classes just structs wihtou methods
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
                        vkMapMemory(device->getLogicalDevice(),vk_deviceMemory,0,size,0,&mapPointer);
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
            public:
                Device *device;
                VkExtent2D extent;

                Image(Device *_device,
                      uint32_t Width,
                      uint32_t Height,
                      VkFormat format,
                      VkImageTiling tiling,
                      VkImageUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      uint32_t num_mips = 1,
                      VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
                      uint32_t arrayLayers = 1,
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
                RefImage get_texture_cached(uint64_t);
                explicit ResourcesManager(Device *device);
                SyncStructure async_loader;
                SyncStructure async_loader2;
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
                * @brief
                * @param device
                * @param width
                * @param height
                * @param format
                * @param tilling
                * @param usage
                * @param properties
                * @param flags
                * @param num_mips
                * @param aspectFlags
                * @param arrayLayers
                * @param samples
                * @return
                */
                ResourceAccessor create_image(   Device *device,
                                                 uint32_t width,
                                                 uint32_t height,
                                                 VkFormat format,
                                                 VkImageTiling tilling,
                                                 VkImageUsageFlags usage,
                                                 VkMemoryPropertyFlags properties,
                                                 VkImageCreateFlags flags,
                                                 uint32_t num_mips =1 ,
                                                 VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
                                                 uint32_t arrayLayers =1 ,
                                                 VkSampleCountFlagBits samples= VK_SAMPLE_COUNT_1_BIT,
                                                 bool store_in_textures_cache = false);


                ResourceAccessor create_texture(const std::string& file_path,Device* device,VkFormat format);

                void generate_mips(std::shared_ptr<Image> image,VkCommandBuffer command,uint32_t width,uint32_t height,uint32_t mip_levels);

                ~ResourcesManager() = default;

                /**
                 * @brief Creates a Device Local Buffer
                 * @tparam Data
                 * @param size
                 * @param flags
                 * @param transfer_pool
                 * @param data
                 * @return
                 */
                uint64_t create_storage_buffer(size_t size, VkBufferUsageFlags flags, VulkanBackend::Commandpool *transfer_pool, void *data);
                /**
                 * @brief
                 * @param command_buffer
                 * @param src
                 * @param dst
                 * @param device_size
                 * @param queue
                 */
                void copy_buffer_to(VkCommandBuffer command_buffer, const RefBuffer &src, const RefBuffer &dst, size_t device_size, VkQueue queue);
                /**
                 * @brief
                 * @param descriptorset
                 * @param buffer_infos
                 * @param image_infos
                 * @return
                 */
                uint64_t write_descriptor_sets(VkDescriptorSet &descriptorset, std::vector<VkDescriptorBufferInfo> &buffer_infos, std::vector<VkDescriptorImageInfo> &image_infos, bool);

                /**
                 * @brief destroys all the resources cached
                 */
                void destroy_non_persistent_resources();

                /**
                 * @brief destory all textures cached
                 */
                void destroy_texture_resources();

                // Events
                Event<VkSemaphore&,VkPipelineStageFlagBits> OnTransferCommand;
                Commandpool resource_loader_pool;

            private:
                std::unordered_map<ResourceAccessor, RefGPUResource> resource_cache;
                //Saving the textures in a different cache because textures don't get recreated in ReloadEvents...
                std::unordered_map<ResourceAccessor, RefGPUResource> textures_cache;
                Device *device;
                std::mutex mutex;

                void copy_buffer_to_image( int tex_width, int tex_height, std::shared_ptr<Buffer> &staging_buffer,
                                           std::shared_ptr<Image> &image, VkCommandBuffer command_buffer ) const;
            };
        }
    } // namespace name

}

#endif