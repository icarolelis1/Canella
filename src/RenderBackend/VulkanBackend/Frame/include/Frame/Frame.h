#pragma once
#ifndef FRAME
#define FRAME

#include "Commandpool/Commandpool.h"
#include "Device/Device.h"
#include "Swapchain/Swapchain.h"
namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            class FrameData
            {

                Device *device;

            public:
                Commandpool commandPool;
                VkFence imageAvaibleFence;
                VkCommandBuffer commandBuffer;
                VkSemaphore imageAcquiredSemaphore;
                VkSemaphore renderFinishedSemaphore;
                FrameData();
                uint32_t beginCommand(Device &device, Swapchain *swapChain, VkCommandBufferUsageFlags usageFlags);
                void finishCommand();
                void build(Device *device);
                void destroy();
            };
        }
    }
}

#endif