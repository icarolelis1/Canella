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

            public:
                Swapchain() = default;
                ~Swapchain() = default;
                Swapchain(const Swapchain &other) = delete;

                void prepare_swapchain(uint32_t WIDTH,
                                       uint32_t HEIGHT,
                                       Device &device,
                                       VkSurfaceKHR surface,
                                       ImageFormat desiredFormat,
                                       GLFWwindow *window,
                                       QueueSharingMode queueSharingMode);

                void destroy_swapchain(Device &device);
                std::vector<VkImageView> &getViews();
                VkExtent2D getExtent();
                VkViewport get_view_port();
                VkRect2D get_rect2d();
                int8_t get_number_of_images();
                VkFormat getFormat();
                VkSwapchainKHR &get_swap_chain_handle();
            private:
                SwapchainProperties properties;

                void create_swapchain(Device &device, Surface &surface, QueueSharingMode &queueSharingMode);
                void aquire_swapchain_images(Device &device);
                void create_swapchain_views(Device &device);
                bool query_swapchain_properties(VkPhysicalDevice device, VkSurfaceKHR surface, VkFormat &desiredFormat, GLFWwindow *window);

                VkSurfaceFormatKHR chooseFormat(std::vector<VkSurfaceFormatKHR> formats, VkFormat desiredFormat);
                VkExtent2D chooseImageExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);
                VkPresentModeKHR choosePresentationMode(std::vector<VkPresentModeKHR> &presentModes);

                uint32_t width;
                uint32_t height;
                std::vector<VkImage> vk_images;
                std::vector<VkImageView> vk_imageViews;
                std::vector<VkDeviceMemory> vk_deviceMemories;
            };
        };

    }
}
#endif