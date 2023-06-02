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
                void build(Device *device);
                void rebuild();
                void destroy();
                Commandpool commandPool;
                Commandpool secondaryPool;
                VkFence imageAvaibleFence;
                VkCommandBuffer commandBuffer;
                VkCommandBuffer editor_command;
                VkSemaphore imageAcquiredSemaphore;
                VkSemaphore renderFinishedSemaphore;
            private:
                Device *device;
            };
        }
    }
}

#endif