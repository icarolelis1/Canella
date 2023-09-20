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
            struct SupportedExtensions
            {
                bool mesh_shader;
                bool storage_16_bit;
                bool min_max_sampler;
                bool push_descriptor;
                bool bindless_suported;
            };
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
            public:
                Device();
                ~Device() = default;
                VkFormat get_depth_supported_format(const std::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags);
                QueueSharingMode getQueueSharingMode();
                VkDevice &getLogicalDevice();
                VkDevice *getLogicalDevicePtr();
                VkPhysicalDevice &getPhysicalDevice();
                VkPhysicalDevice *getPhysicalDevicePtr();
                uint32_t getGraphicsQueueIndex() const;
                VkQueue getGraphicsQueueHandle() const;
                VkQueue  getTransferQueueHandle() const;
                VkQueue getComputeQueueHandle() const;
                uint32_t getPresentQueueIndex()  const;
                const VkAllocationCallbacks* getAllocator();
                uint32_t getTransferQueueIndex() const;
                uint32_t getComputeQueueIndex() const;
                bool prepareDevice(VkSurfaceKHR surface, Instance instance);
                void destroyDevice();
                VkFormat get_depth_format();
                std::mutex& get_queue_mutex(int code);
                float timestamp_period;

            private:
                SupportedExtensions supported_extensions;
                bool bindless_suported;
                bool mesh_shader_supported;
                VkDevice logicalDevice;
                VkPhysicalDevice physicalDevice;
                VkQueue graphicsQueue;
                VkQueue presentationQueue;
                VkQueue transferQueue;
                VkQueue computeQueue;

                std::mutex graphics_mutex;
                std::mutex transfer_mutex;
                std::mutex compute_mutex;
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
                void chain_extensions(VkPhysicalDeviceFeatures2& features);
                bool getQueuFamilieIndex(VkQueueFamilyProperties props, VkQueueFlagBits queueFlags);
                bool querySwapChainProperties(VkPhysicalDevice device, Surface surface);
                void createLogicalDevice();
                VkFormat  depth_format;
                size_t getMinimumBufferAligment();
                std::vector<const char*> deviceExtensions;

            };
        }
    }
}
#endif