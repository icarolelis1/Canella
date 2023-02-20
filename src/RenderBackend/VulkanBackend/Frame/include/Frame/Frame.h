#pragma once
#ifndef FRAME
#define FRAME

#include "Commandpool/Commandpool.h"
namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            class Frame
            {

                Commandpool* commandPool;
                VkFence imageAvaibleFence;
                VkSemaphore imageAcquiredSemaphore;
                VkSemaphore renderFinishedSemaphore;
                Device * device;
            public:
                Frame(Device* device,Commandpool * commandPool);
                ~Frame();
            };
        }
    }
}

#endif