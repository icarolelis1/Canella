#include "Instance/Instance.h"

namespace RenderSystem
{
    namespace VulkanBackend
    {
        DebugLayers::DebugLayers()
        {
            debugMessenger = 0;
        }

        const std::vector<const char *> getValidationLayers()
        {
            return validationLayers;
        }

        const std::vector<const char *> DebugLayers::getExtension(bool enableValidationLayers)
        {
            uint32_t glfwExtensionsCount = 0;
            const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
            std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionsCount);
            if (enableValidationLayers)
            {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // DISPLAY WARNING\ERROR MESSAGES
            }

            return extensions;
        }
        void DebugLayers::setDebugerMessenger(VkDebugUtilsMessengerCreateInfoEXT &createInfo, VkInstance instance)
        {
            populateDebugMessengerCreateInfo(createInfo);

            if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
            {
                std::cout << "    Failed to create DebugMessenger\n";
            }
        }
        void DebugLayers::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
        {
            createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            createInfo.pfnUserCallback = debugCallback;
        }
        bool DebugLayers::checkLayerSupport()
        {
            uint32_t layerCount = 0;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> avaibleLayers;
            avaibleLayers.resize(layerCount);

            vkEnumerateInstanceLayerProperties(&layerCount, avaibleLayers.data());

            // Check if all layer inside validationLayer are supported (inside avaibleLayers).
            // Will tag if it can't find any layer;
            bool anyMyssingLayer = 0;
            for (const char *layerName : validationLayers)
            {
                bool foundLayer = 0;

                for (const auto &layerProperties : avaibleLayers)
                {

                    if (strcmp(layerProperties.layerName, layerName) == 0)
                    {
                        foundLayer = 1;
                        break;
                    }
                }

                if (foundLayer == 0)
                {
                    anyMyssingLayer = 1;
                }
            }

            if (anyMyssingLayer)
            {
                std::cout << "    LAYER NOT SUPPORTED \n";
                return false;
            }

            return true;
        }
        void DebugLayers::destroy(VkInstance instance)
        {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
            std::cout << "    Debuger Destroyed\n";
        };

        Instance::Instance(DebugLayers &debugger, bool enableValidationLayers)
        {
            VkInstanceCreateInfo instanceInfo{};
            instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

            // AplicationInfo;
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Kalm";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "Kalm Render";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_2;
            instanceInfo.pApplicationInfo = &appInfo;
            // Require vulkan validation layers
            auto extensions = debugger.getExtension(enableValidationLayers);
            instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            instanceInfo.ppEnabledExtensionNames = extensions.data();

            instanceInfo.enabledLayerCount = 0;
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
            auto layers = validationLayers;

            if (enableValidationLayers)
            {
                // Layers
                instanceInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
                instanceInfo.ppEnabledLayerNames = layers.data();
                debugger.populateDebugMessengerCreateInfo(debugCreateInfo);
                instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
            }

            if (vkCreateInstance(&instanceInfo, nullptr, &handle) != VK_SUCCESS)
            {
                std::cout << "Failed to create instance\n";
            }
        }

    }

}