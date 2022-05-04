#pragma once
#ifndef DEVICE
#define DEVICE
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include <iostream>
#include "Instance/Instance.h"
#include <set>
#include <string>
#include <optional>
#include <functional>

namespace RenderSystem
{
    namespace VulkanBackend
    {
        struct SwapchainQueryProperties
        {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentationModes;
        };
        struct QueueSharingMode
        {

            VkSharingMode sharingMode;
            uint32_t queueFamiliyIndexCount;
            const uint32_t *queueFamilies;
        };
        struct QueueFamilyIndices
        {
            // Queue Family Indices
            std::optional<uint32_t> transfer;
            std::optional<uint32_t> graphics;
            std::optional<uint32_t> compute;
            std::optional<uint32_t> present;

            bool isComplete()
            {
                checkGraphicsQueue();
                checkTransferQueue();
                checkComputeQueue();

                return graphics.has_value() * transfer.has_value() * compute.has_value() * transfer.has_value();
            }

            bool checkGraphicsQueue()
            {

                if (!graphics.has_value())
                {
                    std::cout << "\tThere is no GraphicsQueue Suitable\n";
                    return false;
                }
                std::cout << "\tThe Graphics Queue is at Index : " << graphics.value() << std::endl;
                return true;
            }

            bool checkTransferQueue()
            {
                if (!transfer.has_value())
                {
                    std::cout << "\nThere is no TransferQueue Suitable\n";
                    return false;
                }
                std::cout << "\tThe Transfer Queue is at Index : " << transfer.value() << std::endl;

                return true;
            }

            bool checkComputeQueue()
            {
                if (!compute.has_value())
                {
                    std::cout << "\nThere is no Compute Suitable\n";
                    return false;
                }
                std::cout << "\tThe Compute Compute is at Index : " << compute.value() << std::endl;

                return true;
            }

            bool checkPresentQueue()
            {
                if (!transfer.has_value())
                {
                    std::cout << "\nThere is no Presentation Suitable\n";
                    return false;
                }
                std::cout << "\tThe Presentation Queue is at Index : " << present.value() << std::endl;

                return true;
            }
        };

        const std::vector<const char *> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        using Surface = VkSurfaceKHR;
        class Device
        {
            VkDevice logicalDevice;
            VkPhysicalDevice physicalDevice;
            VkQueue graphicsQueue;
            VkQueue presentationQueue;
            VkQueue transferQueue;
            QueueFamilyIndices queueFamilies;
            VkPhysicalDeviceProperties vk_physicalDeviceProperties;
            VkPhysicalDeviceMemoryProperties vk_MemoryProperties;
            VkPhysicalDeviceFeatures vk_PhysicalDevicefeatures;

            VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

            void choosePhysicalDevice(Instance instance, Surface surface);
            bool checkDeviceExtensions(VkPhysicalDevice device);
            VkSampleCountFlagBits getMaxUsableSampleCount();
            int scorePhysicalDevice(VkPhysicalDevice device, VkPhysicalDeviceFeatures features, VkPhysicalDeviceMemoryProperties memProperties, Surface surface);
            bool getQueuFamilieIndex(VkQueueFamilyProperties props, VkQueueFlagBits queueFlags);
            bool querySwapChainProperties(VkPhysicalDevice device, Surface surface);
            void createLogicalDevice();
            size_t getMinimumBufferAligment();
            QueueSharingMode getQueueSharingMode();
            void destroyDevice();
        public:
            Device();
            ~Device();
            VkDevice& getLogicalDevice();
        };
    }
}

#endif