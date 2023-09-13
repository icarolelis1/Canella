#include "AsynchronousLoader/SyncStructure.h"

using namespace Canella::RenderSystem::VulkanBackend;

SyncStructure::SyncStructure( Device *_device) : device( _device) {}

void SyncStructure::destroy()
{
    vkDestroySemaphore(device->getLogicalDevice(), semaphore, device->getAllocator());
    vkDestroySemaphore(device->getLogicalDevice(), wait_semaphore, device->getAllocator());
    vkDestroyFence(device->getLogicalDevice(), fence, device->getAllocator());
}

void SyncStructure::build()
{
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device->getLogicalDevice(),&semaphore_info, nullptr, &semaphore) != VK_SUCCESS ||
        vkCreateSemaphore(device->getLogicalDevice(),&semaphore_info, nullptr, &wait_semaphore) != VK_SUCCESS ||
        vkCreateFence(device->getLogicalDevice(), &fence_info, nullptr, &fence))
        Logger::Error("Failed to create synchronization objects for Resource Loader");
}
