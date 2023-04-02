#pragma once
#ifndef DEVICE
#define DEVICE
#include "Logger/Logger.hpp"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <optional>
#include <vector>
#include <iostream>
#include "Instance/Instance.h"
#include <set>
#include <string>
#include <optional>
#include <functional>
#include <vector>
namespace Canella
{
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
                    checkPresentQueue();
                    return graphics.has_value() * transfer.has_value() * compute.has_value() * transfer.has_value();
                }

                bool checkGraphicsQueue()
                {

                    if (!graphics.has_value())
                    {
                        Canella::Logger::Error("There is no Graphics Queue Suitable");
                        return false;
                    }
                    Canella::Logger::Info("The Graphics Queue is at Index : %d", graphics.value());
                    return true;
                }

                bool checkTransferQueue()
                {
                    if (!transfer.has_value())
                    {
                        Canella::Logger::Error("There is no Transfer Queue Suitable");
                        return false;
                    }
                    Canella::Logger::Info("The Transfer Queue is at Index : %d", transfer.value());

                    return true;
                }

                bool checkComputeQueue()
                {
                    if (!compute.has_value())
                    {
                        Canella::Logger::Error("There is no Compute Queue Suitable");
                        return false;
                    }
                    Canella::Logger::Info("The Compute Queue is at Index : %d", compute.value());

                    return true;
                }

                bool checkPresentQueue()
                {
                    if (!transfer.has_value())
                    {
                        Canella::Logger::Error("There is no Presentation Queue Suitable");
                        return false;
                    }
                    Canella::Logger::Info("The Present Queue is at Index : %d", present.value());
                    return true;
                }
            };
            
            using Surface = VkSurfaceKHR;
            class Device
            {
                bool bindless_suported;
                bool mesh_shader_supported;
                VkDevice logicalDevice;
                VkPhysicalDevice physicalDevice;
                VkQueue graphicsQueue;
                VkQueue presentationQueue;
                VkQueue transferQueue;
                VkQueue computeQueue;
                QueueFamilyIndices queueFamilies;
                VkPhysicalDeviceProperties vk_physicalDeviceProperties;
                VkPhysicalDeviceMemoryProperties vk_MemoryProperties;
                VkPhysicalDeviceFeatures vk_PhysicalDevicefeatures;
                VkPhysicalDeviceFeatures2 vk_PhysicalDevicefeatures2;
                VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
                void choosePhysicalDevice(Instance instance, Surface surface);
                bool checkDeviceExtensions(VkPhysicalDevice device);
                VkSampleCountFlagBits getMaxUsableSampleCount();
                int scorePhysicalDevice(VkPhysicalDevice device, VkPhysicalDeviceFeatures features, VkPhysicalDeviceMemoryProperties memProperties, Surface surface);
                bool getQueuFamilieIndex(VkQueueFamilyProperties props, VkQueueFlagBits queueFlags);
                bool querySwapChainProperties(VkPhysicalDevice device, Surface surface);
                void enableMeshShaderExtension();
                void createLogicalDevice();
                size_t getMinimumBufferAligment();
                std::vector<const char*> deviceExtensions;
                VkPhysicalDeviceDescriptorIndexingFeatures indexing_features{};
                VkPhysicalDeviceMeshShaderFeaturesEXT enabledMeshShaderFeatures{};
            public:
                Device();
                QueueSharingMode getQueueSharingMode();
                VkDevice &getLogicalDevice();
                VkDevice *getLogicalDevicePtr();
                VkPhysicalDevice &getPhysicalDevice();
                VkPhysicalDevice *getPhysicalDevicePtr();
                bool prepareDevice(VkSurfaceKHR surface, Instance instance);
                uint32_t getGraphicsQueueIndex() const;
                VkQueue getGraphicsQueueHandle() const;
                uint32_t getTransferQueueIndex() const;
                uint32_t getComputeQueueIndex()  const;
                uint32_t getPresentQueueIndex()  const;
                const VkAllocationCallbacks* getAllocator();
                ~Device();
                void destroyDevice();
            };
        }
    }
}
#endif