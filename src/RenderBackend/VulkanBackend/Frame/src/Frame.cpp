#include "Frame/Frame.h"
#include "Logger/Logger.hpp"
namespace Canella
{
	namespace RenderSystem
	{
		namespace VulkanBackend
		{

			FrameData::FrameData() : commandPool()
			{
			}

			/**
			 * @brief
			 * FrameData object corresponds to a swapchain image
			 * Each FrameData has a collection of syncronization primitives that allow
			 * Record overlapping between frames and a set of commandspools
			 * @param _device
			 */
			void FrameData::build(Device *device)
			{
				this->device = device;
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
				else
				{
					Logger::Debug("Succesfully  created FrameData");
				}

				commandPool.build(device, POOL_TYPE::GRAPHICS, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
				commandBuffer = commandPool.requestCommandBuffer(device, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
			}

			uint32_t FrameData::beginCommand(Device& device,Swapchain* swapChain, VkCommandBufferUsageFlags usageFlags)
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
		}
	}
}