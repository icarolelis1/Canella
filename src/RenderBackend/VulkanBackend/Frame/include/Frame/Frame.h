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
            public:
                FrameData() = default;
                uint32_t begin_command(Device &device, Swapchain *swapChain, VkCommandBufferUsageFlags usageFlags);
                void finishCommand();
                void build(Device *device);
                void destroy();
                
                Commandpool commandPool;
                VkFence imageAvaibleFence;
                VkCommandBuffer commandBuffer;
                VkSemaphore imageAcquiredSemaphore;
                VkSemaphore renderFinishedSemaphore;
            private:
                Device *device;
            };
        }
    }
}

#endif