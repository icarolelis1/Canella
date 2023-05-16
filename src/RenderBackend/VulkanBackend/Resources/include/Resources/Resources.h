#ifndef VK_RESOURCES
#define VK_RESOURCES
#include <Meshoptimizer/meshoptimizer.h>
#include "Device/Device.h"
#include <unordered_map>

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
                void destroy(Device &device) const;

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
            using RefDescriptorset = std::shared_ptr<DescriptorSet>;

            /**
            * \brief Manages vulkan resources
            */
            class ResourcesManager{
            private:
                std::unordered_map<ResourceAccessor,RefGPUResource> resource_cache;
                std::unordered_map<ResourceAccessor,VkDescriptorSet> descriptorset_cache;
                Device* device;

            public:
                RefBuffer get_buffer_cached(uint64_t);
                explicit ResourcesManager(Device* device);
                ResourceAccessor create_buffer(VkDeviceSize size,
                                               VkBufferUsageFlags usage,
                                               VkMemoryPropertyFlags properties);

                void allocate_resource(
                                            ResourceAccessor,
                                            std::vector<VkDescriptorBufferInfo>&,
                                            std::vector<VkDescriptorImageInfo>&);
                ~ResourcesManager();
            };
        }
    } // namespace name

}

#endif