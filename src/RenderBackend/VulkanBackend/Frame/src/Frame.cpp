#include "Frame/Frame.h"
#include "Logger/Logger.hpp"
namespace Canella
{
	namespace RenderSystem
	{
		namespace VulkanBackend
		{
			/**
			 * @brief
			 * FrameData object corresponds to a swapchain image
			 * Each FrameData has a collection of syncronization primitives that allow
			 * Record overlapping between frames and a set of commandspools
			 * @param device
			 */
			void FrameData::build(Device *device)
			{
				this->device = device;
				VkSemaphoreCreateInfo semaph_info{};
				semaph_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

				VkFenceCreateInfo fence_info{};
				fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

				if (vkCreateSemaphore(device->getLogicalDevice(), &semaph_info, nullptr, &imageAcquiredSemaphore) != VK_SUCCESS ||
					vkCreateSemaphore(device->getLogicalDevice(), &semaph_info, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
					vkCreateFence(device->getLogicalDevice(), &fence_info, nullptr, &imageAvaibleFence))
				{

					Logger::Error("Failed to create synchronization objects for given frame");
				}
				else
				{
					Logger::Debug("Succesfully  created FrameData");
				}

				commandPool.build(device, POOL_TYPE::GRAPHICS, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
				commandBuffer = commandPool.requestCommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
                editor_command = commandPool.requestCommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			}

			/**
			 * \brief 
			 * \param device 
			 * \param swapChain 
			 * \param usageFlags 
			 * \return 
			 */
			uint32_t FrameData::begin_command(Device& device,Swapchain* swapChain, VkCommandBufferUsageFlags usageFlags)
			{
				return 0;
			}

			void FrameData::destroy()
			{
				vkDestroySemaphore(device->getLogicalDevice(), imageAcquiredSemaphore, device->getAllocator());
				vkDestroySemaphore(device->getLogicalDevice(), renderFinishedSemaphore, device->getAllocator());
				vkDestroyFence(device->getLogicalDevice(), imageAvaibleFence, device->getAllocator());
				Logger::Info("Destroyed Syncronization objects");
				commandPool.destroy(device);
			}

            void FrameData::rebuild() {

                vkResetCommandPool(device->getLogicalDevice(),
                                   commandPool.pool,
                                   VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

                vkDestroySemaphore(device->getLogicalDevice(), imageAcquiredSemaphore, device->getAllocator());
                vkDestroySemaphore(device->getLogicalDevice(), renderFinishedSemaphore, device->getAllocator());
                vkDestroyFence(device->getLogicalDevice(), imageAvaibleFence, device->getAllocator());
                Logger::Info("Destroyed Syncronization objects");

                VkSemaphoreCreateInfo semaph_info{};
                semaph_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

                VkFenceCreateInfo fence_info{};
                fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

                if (vkCreateSemaphore(device->getLogicalDevice(), &semaph_info, nullptr, &imageAcquiredSemaphore) != VK_SUCCESS ||
                    vkCreateSemaphore(device->getLogicalDevice(), &semaph_info, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
                    vkCreateFence(device->getLogicalDevice(), &fence_info, nullptr, &imageAvaibleFence))
                {

                    Logger::Error("Failed to create synchronization objects for given frame");
                }
                else
                {
                    Logger::Debug("Succesfully  created FrameData");
                }

            }
        }
	}
}