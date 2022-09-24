#pragma once
#ifndef COMMAND_POOL
#define COMMAND_POOL

/**
 * @file Commandpool.h
 * @author your name (you@domain.com)
 * @brief Command pool that allocates commandbuffer
 * @version 0.1
 * @date 2022-09-08
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "Device/Device.h"

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            /**
             * @brief Enumerator of queue types
             * 
             */
            enum class POOL_TYPE
            {
                GRAPHICS,
                COMPUTE,
                TRANSFER
            };

            class Commandpool
            {

                VkCommandPool pool;
                Device *device;

                ~Commandpool();

            public:
                /**
                 * @brief Construct a new Commandpool objectc
                 * 
                 * @param _device 
                 * @param type 
                 * @param flags 
                 */
                Commandpool( Device * _device, POOL_TYPE type, VkCommandPoolCreateFlags flags);

                /**
                 * @brief Request a new command from the command pool (internally allocates it)
                 * 
                 * @param level wheter the commmand is primary or secondary
                 * @return VkCommandBuffer commandbuffer allocated
                 */
                VkCommandBuffer requestCommandBuffer(VkCommandBufferLevel level) const;
                Commandpool(Commandpool &) = delete;

                /**
                 * @brief allocate  acommand buffer
                 * 
                 * @param cmdBuffers 
                 * @param level 
                 */
                void allocateCommandBuffer(std::vector<VkCommandBuffer> &cmdBuffers, VkCommandBufferLevel level) const;
                /**
                 * @brief allocates a bunch of commands
                 * 
                 * @param cmdBuffer 
                 * @param level 
                 */
                void allocateCommandBuffer(VkCommandBuffer &cmdBuffer, VkCommandBufferLevel level) const;

                /**
                 * @brief call beginCommandbuffer on the command, changing it's state to RECORDING
                 * 
                 * @param buffer 
                 * @param single_usage Whether the command is single_usage or not (This can be used to get performance)
                 */
                void beginCommandBuffer(VkCommandBuffer& buffer, bool single_usage = false);

                /**
                 * @brief Calls vkEndCommandBuffer on the commandBuffer changing it's state to PENDING again
                 * 
                 * @param buffer the buffer to be ended
                 */
                void endCommandBuffer(VkCommandBuffer& buffer);
            };

        }
    }
}
#endif