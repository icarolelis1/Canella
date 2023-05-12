#pragma once
#ifndef VULKAN_SWAPCHAIN
#define VULKAN_SWAPCHAIN
#include "Device/Device.h"

#include <Window/Window.h>

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            using ImageFormat = VkFormat;
            struct SwapchainProperties
            {

                VkSurfaceFormatKHR format;
                VkPresentModeKHR presentMode;
                VkSurfaceCapabilitiesKHR capabilities;
                uint32_t minImageCount;
                VkExtent2D extent;
                VkSharingMode sharingMode;
                VkSwapchainKHR vk_swapChain;
            };

            class Swapchain
            {
            private:
                SwapchainProperties properties;

                void createSwapChain(Device &device, Surface &surface, QueueSharingMode &queueSharingMode);
                void aquireSwapChainImages(Device &device);
                void createSwapchainViews(Device &device);
                bool querySwapChainProperties(VkPhysicalDevice device, VkSurfaceKHR surface, VkFormat &desiredFormat, GLFWwindow *window);

                VkSurfaceFormatKHR chooseFormat(std::vector<VkSurfaceFormatKHR> formats, VkFormat desiredFormat);
                VkExtent2D chooseImageExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);
                VkPresentModeKHR choosePresentationMode(std::vector<VkPresentModeKHR> &presentModes);

                uint32_t width;
                uint32_t height;
                std::vector<VkImage> vk_images;
                std::vector<VkImageView> vk_imageViews;
                std::vector<VkDeviceMemory> vk_deviceMemories;

            public:
                Swapchain();
                Swapchain(const Swapchain &other) = delete;

                void prepare_swapchain(uint32_t WIDTH,
                                       uint32_t HEIGHT,
                                       Device &device,
                                       VkSurfaceKHR surface,
                                       ImageFormat desiredFormat,
                                       GLFWwindow *window,
                                       QueueSharingMode queueSharingMode);

                void destroySwapchain(Device &device);
                std::vector<VkImage> getImages();
                std::vector<VkImageView> &getViews();
                VkFormat getSwapchainFormat();
                VkExtent2D getExtent();
                VkViewport get_view_port();
                VkRect2D get_rect2d();
                uint32_t getNumberOfImages();
                VkFormat getFormat();
                VkSwapchainKHR &getSwapChainHandle();
            };
        };

    }
}
#endif