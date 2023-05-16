#ifndef VK_RESOURCES
#define VK_RESOURCES
#include <Meshoptimizer/meshoptimizer.h>
#include "Device/Device.h"
#include <unordered_map>
#include "Commandpool/Commandpool.h"
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
                GPUResource(ResourceType type);
                ResourceType type;

            };

            class Buffer : public GPUResource
            {
            public:
                Buffer(Device *_device, VkDeviceSize size,
                       VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
                template <typename T>
                void udpate(T &object);
                VkBuffer &getBufferHandle();
                VkDeviceMemory &getMemoryHandle();

                ~Buffer();

            private:
                bool mapped = false;
                uint32_t find_memory_type(Device *device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
                VkBuffer vk_buffer;
                VkDeviceMemory vk_deviceMemory;
                void* mapPointer;
                Device* device;
            };
            
            template <typename T>
            inline void Buffer::udpate(T &object)
            {
                // Keep persistent mapped buffers
                if (!mapped)
                    vkMapMemory(device->getLogicalDevice(),
                                vk_deviceMemory,
                                0,
                                sizeof(object),
                                0,
                                &mapPointer);

                memcpy(mapPointer, &object, sizeof(object));
                mapped = true;
            }

            void copy_buffer_to(            VkCommandBuffer command_buffer,
                                            Buffer& src,
                                            Buffer& dst,
                                            VkDeviceSize device_size,
                                            VkQueue queue);

            using RefGPUResource = std::shared_ptr<GPUResource>;
            using RefBuffer = std::shared_ptr<Buffer>;

            /**
            * \brief Manages vulkan resources
            */
            class ResourcesManager{
            private:
                std::unordered_map<ResourceAccessor,RefGPUResource> resource_cache;
                Device* device;

            public:
                RefBuffer get_buffer_cached(uint64_t);
                explicit ResourcesManager(Device* device);
                ResourceAccessor create_buffer(VkDeviceSize size,
                                               VkBufferUsageFlags usage,
                                               VkMemoryPropertyFlags properties);
                ~ResourcesManager();

                template<typename Blob>
                uint64_t create_host_visible_buffer(VkDeviceSize size,
                                                    VkBufferUsageFlags flags,
                                                    VulkanBackend::Commandpool* transfer_pool,
                                                    Blob& data)
                {
                    //staging buffer will be destroyed immediatly after transfer
                    auto staging_buffer = Buffer(device,
                                                 size,
                                                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
                    staging_buffer.udpate(data);

                    ResourceAccessor id = create_buffer(size,
                                                        flags,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                    const auto command = transfer_pool->requestCommandBuffer(
                            device,
                            VK_COMMAND_BUFFER_LEVEL_PRIMARY );
                    copy_buffer_to(command,
                                   staging_buffer,
                                   *get_buffer_cached(id).get(),
                                   size,
                                   device->getTransferQueueHandle());
                    return id;
                }


                uint64_t
                write_descriptor_sets(VkDescriptorSet& descriptorset,
                                      std::vector<VkDescriptorBufferInfo> &buffer_infos,
                                      std::vector<VkDescriptorImageInfo> &image_infos);
            };
        }
    } // namespace name

}

#endif