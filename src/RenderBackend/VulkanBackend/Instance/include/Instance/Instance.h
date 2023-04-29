/*
    Contains basic vulkan framework.
    Vulkan Resources (Images and Buffers )  are created in their own files
*/

#pragma once
#ifndef VULKAN_FRAME_WORK
#define VULKAN_FRAME_WORK
#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "Logger/Logger.hpp"

namespace Canella {
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
            {

                auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
                if (func != nullptr)
                {
                    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
                }
                else
                {
                    return VK_ERROR_EXTENSION_NOT_PRESENT;
                }
            }

            inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
            {
                auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
                if (func != nullptr)
                {
                    func(instance, debugMessenger, pAllocator);
                }
            }

            // STANDARD LUNGARG validation layer
            static const std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation",
                
            };

            // Enable validation layers for debugging
            class DebugLayers
            {
                VkDebugUtilsMessengerEXT debugMessenger;
                // Debug messenger callback function
                static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
                    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                    void* pUserData)
                {
                    Logger::Debug("Validation Layer: %s", pCallbackData->pMessage);

                    return VK_FALSE;
                }

            public:
                DebugLayers();

                const std::vector<const char*> getValidationLayers;
                const std::vector<const char*> getExtension(bool enableValidationLayers);
                void setDebugerMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo, VkInstance instance);
                void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
                bool checkLayerSupport();
                void destroy(VkInstance instance);
            };

            class Instance
            {

            public:
                // Create Vulkan Instance
                Instance(DebugLayers& debugger, bool enableValidationLayers);
                VkInstance handle;
            };

        }
    }
}
#endif