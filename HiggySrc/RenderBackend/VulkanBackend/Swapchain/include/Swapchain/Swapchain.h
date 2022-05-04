// #pragma once
// #ifndef VULKAN_SWAPCHAIN
// #define VULKAN_SWAPCHAIN
// #include "Device/Device.h"
// namespace RenderSystem
// {
//     namespace VulkanBackend
//     {
//         using ImageFormat = VkFormat;
//         struct SwapchainProperties
//         {

//             VkSurfaceFormatKHR format;
//             VkPresentModeKHR presentMode;
//             VkSurfaceCapabilitiesKHR capabilities;
//             uint32_t minImageCount;
//             VkExtent2D extent;
//             VkSharingMode sharingMode;
//             VkSwapchainKHR vk_swapChain;
//         };

//         class Swapchain
//         {
//             std::vector<VkImage> getImages();
//             std::vector<VkImageView> &getViews();
//             VkFormat getSwapchainFormat();
//             VkExtent2D getExtent();
//             uint32_t getNumberOfImages();
//             VkFormat getFormat();
//             VkSwapchainKHR &getSwapChainHandle();
//             SwapchainProperties properties;

//             void createSwapChain(Device device, Surface &surface, QueueSharingMode &queueSharingMode);
//             void aquireSwapChainImages(Device &device);
//             void createSwapchainViews(Device &device);
//             bool querySwapChainProperties(Device device, Surface surface, VkFormat &desiredFormat, GLFWwindow *window);
//             VkSurfaceFormatKHR chooseFormat(std::vector<VkSurfaceFormatKHR> formats, ImageFormat &desiredFormat);
//             VkExtent2D chooseImageExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);
//             VkPresentModeKHR choosePresentationMode(std::vector<VkPresentModeKHR> &presentModes);

//         public:
//             Swapchain(uint32_t WIDTH, uint32_t HEIGHT, Device device, Surface surface, VkFormat &desiredFormat, GLFWwindow *window, QueueSharingMode &queueSharingMode);
//             Swapchain();
//             Swapchain(const Swapchain &other) = delete;
//             ~Swapchain();
//             void prepareSwapchain(uint32_t WIDTH, uint32_t HEIGHT, Device device, Surface *surface, VkFormat &desiredFormat, GLFWwindow *window, QueueSharingMode &queueSharingMode);
//             void destroySwapchain(Device &device);
//         };
//     }
// }

// #endif