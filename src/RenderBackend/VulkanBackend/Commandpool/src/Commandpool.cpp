#include "Commandpool/Commandpool.h"
#include "Logger/Logger.hpp"

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            void Commandpool::build(Device *device, POOL_TYPE type, VkCommandPoolCreateFlags flags)
            {
                VkCommandPoolCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

                if (type == POOL_TYPE::GRAPHICS)
                    createInfo.queueFamilyIndex = device->getGraphicsQueueIndex();
                else if (type == POOL_TYPE::COMPUTE)
                    createInfo.queueFamilyIndex = device->getComputeQueueIndex();
                else if (type == POOL_TYPE::TRANSFER)
                    createInfo.queueFamilyIndex = device->getTransferQueueIndex();
               // createInfo.flags = flags;
                if (const VkResult result = vkCreateCommandPool(device->getLogicalDevice(),
                                                                &createInfo,
                                                                nullptr,
                                                                &pool);
                result != VK_SUCCESS) Logger::Debug("Failed to create command pool");
            }

            VkCommandBuffer Commandpool::requestCommandBuffer(Device *device, VkCommandBufferLevel level) const
            {
                VkCommandBuffer buffer;
                allocateCommandBuffer(device, buffer, level);

                return buffer;
            }

            void Commandpool::allocateCommandBuffer(Device *device, VkCommandBuffer &buffer, VkCommandBufferLevel level) const
            {
                VkCommandBufferAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = pool;
                allocInfo.level = level;
                allocInfo.commandBufferCount = static_cast<uint32_t>(1);
                VkResult result = vkAllocateCommandBuffers(device->getLogicalDevice(), &allocInfo, &buffer);
                if (result != VK_SUCCESS) Logger::Error("Failed to allocate CommandBuffer");
            }

            void Commandpool::begin_command_buffer(Device* device, VkCommandBuffer &buffer, bool single_usage)const
            {
                vkResetCommandPool(device->getLogicalDevice(),pool,VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
                VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
                if (single_usage)
                {
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                };
           
                vkBeginCommandBuffer(buffer, &beginInfo);
            }
            void Commandpool::endCommandBuffer(VkCommandBuffer &buffer)
            {
                vkEndCommandBuffer(buffer);
            }

            void Commandpool::destroy(Device *device)
            {
                vkDestroyCommandPool(device->getLogicalDevice(), pool, device->getAllocator());
            }
        }
    }
}