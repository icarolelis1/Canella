#include "Frame/Frame.h"
#include "Logger/Logger.hpp"

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            
            /**
             * @brief Construct a new Frame:: Frame object
             * Frame object corresponds to a swapchain image
             * Each frame has a collection of syncronization primitives that allow
             * Record overlapping between frames and a set of commandspools
             * @param _device 
             */
            Frame::Frame(Device *_device) : device(_device)
            {
                VkSemaphoreCreateInfo semaphInfo{};
                semaphInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

                VkFenceCreateInfo fenceInfo{};
                fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

                if (vkCreateSemaphore(device->getLogicalDevice(), &semaphInfo, nullptr, &imageAcquiredSemaphore) != VK_SUCCESS ||
                    vkCreateSemaphore(device->getLogicalDevice(), &semaphInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
                    vkCreateFence(device->getLogicalDevice(), &fenceInfo, nullptr, &imageAvaibleFence))
                {

                    Logger::Error("Failed to create syncronization objects for given frame");
                }
            }

            Frame::~Frame()
            {
                vkDestroySemaphore(device->getLogicalDevice(), imageAcquiredSemaphore, device->getAllocator());
                vkDestroySemaphore(device->getLogicalDevice(), renderFinishedSemaphore, device->getAllocator());
                vkDestroyFence(device->getLogicalDevice(), imageAvaibleFence, device->getAllocator());
                Logger::Info("Destroyed Syncronization objects");
            }
        }
    }
}