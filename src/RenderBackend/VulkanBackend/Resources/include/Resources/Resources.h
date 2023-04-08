#ifndef VK_RESOURCES
#define VK_RESOURCES
#include "Device/Device.h"

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            class Buffer
            {
            public:
                Buffer(Device *_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
                template <typename T>
                void udpate(T &object);
                bool isMapped();
                VkBuffer &getBufferHandle();
                VkDeviceMemory &getMemoryHandle();
                void destroy(Device &device) const;

            private:
                bool mapped = false;
                uint32_t findMemoryType(Device *device, uint32_t typeFilter, VkMemoryPropertyFlags properties);
                VkBuffer vk_buffer;
                VkDeviceMemory vk_deviceMemory;
                Device* device;
                void* mapPointer;
            };
            template <typename T>
            inline void Buffer::udpate(T &object)
            {
                // Keep persistent mapped buffers
                if (!mapped)
                    vkMapMemory(device->getLogicalDevice(), vk_deviceMemory, 0, sizeof(object), 0, &mapPointer);

                memcpy(mapPointer, &object, sizeof(object));
                mapped = true;
            }
        }

    } // namespace name

}

#endif