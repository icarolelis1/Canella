// #include "Swapchain/Swapchain.h"
// namespace RenderSystem
// {
//     namespace VulkanBackend
//     {
//         Swapchain::Swapchain(){};
//         Swapchain::Swapchain(uint32_t WIDTH, uint32_t HEIGHT, Device device, Surface surface, ImageFormat &desiredFormat, GLFWwindow *window, QueueSharingMode &queueSharingMode)
//         {
//             properties.extent.width = WIDTH;
//             properties.extent.height = HEIGH;
//             querySwapChainProperties(device, surface, desiredFormat, window);
//             createSwapChain(device, surface, queueSharingMode);
//             aquireSwapChainImages(device);
//             createSwapchainViews(device);
//         };
//         void Swapchain::prepareSwapchain(uint32_t WIDTH, uint32_t HEIGHT, Device device, Surface *surface, ImageFormat &desiredFormat, GLFWwindow *window, QueueSharingMode &queueSharingMode)
       
//             querySwapChainProperties(device, *surface, desiredFormat, window);
//             createSwapChain(device, *surface, queueSharingMode);
//             aquireSwapChainImages(device);
//             createSwapchainViews(device);
       
//         void Swapchain::destroySwapchain(Device &device)
//         {
            
//             for (auto view : swapChainImages.vk_imageViews)
//             {
//                 vkDestroyImageView(device.getLogicalDevice(), view, nullptr);
           
//             vkDestroySwapchainKHR(device.getLogicalDevice(), properties.vk_swapChain, nullptr);
//             std::cout << "    Successfully destroyed SwapChain\n";
//         }
//     }
// }