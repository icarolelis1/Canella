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
                FrameData(const FrameData&other ) = default;
                /**
                 * @brief
                 * FrameData object corresponds to a swapchain image
                 * Each FrameData has a collection of syncronization primitives that allow
                 * Record overlapping between frames and a set of commandspools
                 * @param device
                 */
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