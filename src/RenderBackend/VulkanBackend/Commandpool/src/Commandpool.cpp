#include "Commandpool/Commandpool.h"
#include "Logger/Logger.hpp"

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            Commandpool::Commandpool(Device *_device, POOL_TYPE type, VkCommandPoolCreateFlags flags) : device(_device)
            {
                VkCommandPoolCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

                if (type == POOL_TYPE::GRAPHICS)
                    createInfo.queueFamilyIndex = device->getGraphicsQueueIndex();
                else if (type == POOL_TYPE::COMPUTE)
                    createInfo.queueFamilyIndex = device->getComputeQueueIndex();
                else if (type == POOL_TYPE::TRANSFER)
                    createInfo.queueFamilyIndex = device->getTransferQueueIndex();
                createInfo.flags = flags;
                VkResult result = vkCreateCommandPool(device->getLogicalDevice(), &createInfo, nullptr, &pool);
            }

            VkCommandBuffer Commandpool::requestCommandBuffer(VkCommandBufferLevel level) const
            {
                VkCommandBuffer buffer;
                allocateCommandBuffer(buffer, level);

                return buffer;
            }

            void Commandpool::allocateCommandBuffer(VkCommandBuffer &buffer, VkCommandBufferLevel level) const
            {
                VkCommandBufferAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool = pool;
                allocInfo.level = level;
                allocInfo.commandBufferCount = static_cast<uint32_t>(1);
                // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_

                VkResult result = vkAllocateCommandBuffers(device->getLogicalDevice(), &allocInfo, &buffer);
                if (result != VK_SUCCESS)
                    Logger::Error("Failed to allocate CommandBuffer");
            }

            void Commandpool::beginCommandBuffer(VkCommandBuffer &buffer, bool single_usage)
            {
                VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
                if (single_usage)
                {
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                };
            }
            void Commandpool::endCommandBuffer(VkCommandBuffer &buffer)
            {
                vkEndCommandBuffer(buffer);
            }

            Commandpool::~Commandpool()
            {
                vkDestroyCommandPool(device->getLogicalDevice(), pool, device->getAllocator());
            }
        }
    }
}