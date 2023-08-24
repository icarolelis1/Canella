#include "Device/Device.h"
#include "Logger/Logger.hpp"

//TODO CLEAN THIS FILE
namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            Device::Device()
            {
                deviceExtensions.push_back("VK_KHR_swapchain");
                deviceExtensions.push_back("VK_KHR_push_descriptor");
                deviceExtensions.push_back("VK_KHR_16bit_storage");
                deviceExtensions.push_back("VK_KHR_draw_indirect_count");
                deviceExtensions.push_back("VK_EXT_sampler_filter_minmax");
                deviceExtensions.push_back("VK_EXT_mesh_shader");
                deviceExtensions.push_back("VK_KHR_spirv_1_4");
                deviceExtensions.push_back("VK_KHR_shader_float_controls");
                deviceExtensions.push_back("VK_KHR_maintenance1");
            };

            void Device::choosePhysicalDevice(Instance instance, Surface surface)
            {
                uint32_t deviceCount = 0;
                vkEnumeratePhysicalDevices(instance.handle, &deviceCount, nullptr);

                if (deviceCount == 0)
                    Logger::Error("Failed to find a proper Graphics Card");

                std::vector<VkPhysicalDevice> devices(deviceCount);
                vkEnumeratePhysicalDevices(instance.handle, &deviceCount, devices.data());

                VkBool32 findSuitableGPU = false;

                int currentScore = 0;
                int bestScore = 0;
                VkPhysicalDevice bestDevice = NULL;

                for (const auto& device : devices)
                {
                    VkPhysicalDeviceProperties physicalDeviceProps;
                    VkPhysicalDeviceFeatures features;
                    VkPhysicalDeviceMemoryProperties props;
                    // Query physical device properties
                    vkGetPhysicalDeviceProperties(device, &physicalDeviceProps);
                    vkGetPhysicalDeviceMemoryProperties(device, &props);
                    vkGetPhysicalDeviceFeatures(device, &features);
                    timestamp_period = physicalDeviceProps.limits.timestampPeriod;
                    currentScore = scorePhysicalDevice(device, features, props, surface);;
                    if (currentScore >=200)
                    {
                        findSuitableGPU = true;
                        bestDevice = device;
                        break;
                    }
                }

                if (!findSuitableGPU)
                {
                    Logger::Error("Device don't support required extensions or is not a discrete GPU Score %d",currentScore);
                    return;
                }

                physicalDevice = bestDevice;
                // Query physical device properties
                vkGetPhysicalDeviceProperties(physicalDevice, &vk_physicalDeviceProperties);
                vkGetPhysicalDeviceMemoryProperties(physicalDevice, &vk_MemoryProperties);
                vkGetPhysicalDeviceFeatures(physicalDevice, &vk_PhysicalDevicefeatures);

                physicalDevice = bestDevice;
                msaaSamples = getMaxUsableSampleCount();
                Logger::Info("Device %s ", vk_physicalDeviceProperties.deviceName);
            }

            bool Device::checkDeviceExtensions(VkPhysicalDevice device)
            {
                uint32_t extensionsCount;
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

                std::vector<VkExtensionProperties> extensions(extensionsCount);
                vkEnumerateDeviceExtensionProperties(device, nullptr,
                                                     &extensionsCount, extensions.data());

                auto required_ext_count = 0;
                auto contains_extension = [&](const char* ext)
                {
                    for(auto& required_ext: deviceExtensions)
                        if(strcmp(required_ext,ext) == 0)
                            return true;
                    return false;
                };

                for (const auto& extension : extensions)
                {
                    required_ext_count = contains_extension(extension.extensionName)? required_ext_count+1:required_ext_count;
                }

                return required_ext_count == deviceExtensions.size();
            }

            VkSampleCountFlagBits Device::getMaxUsableSampleCount()
            {
                VkPhysicalDeviceProperties physicalDeviceProperties;
                vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

                VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                    physicalDeviceProperties.limits.framebufferDepthSampleCounts;
                if (counts & VK_SAMPLE_COUNT_64_BIT)
                {
                    return VK_SAMPLE_COUNT_64_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_32_BIT)
                {
                    return VK_SAMPLE_COUNT_32_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_16_BIT)
                {
                    return VK_SAMPLE_COUNT_16_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_8_BIT)
                {
                    return VK_SAMPLE_COUNT_8_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_4_BIT)
                {
                    return VK_SAMPLE_COUNT_4_BIT;
                }
                if (counts & VK_SAMPLE_COUNT_2_BIT)
                {
                    return VK_SAMPLE_COUNT_2_BIT;
                }

                return VK_SAMPLE_COUNT_1_BIT;
            }

            int Device::scorePhysicalDevice(VkPhysicalDevice device, VkPhysicalDeviceFeatures features,
                                            VkPhysicalDeviceMemoryProperties memProperties, Surface surface)
            {
                int score = 200;
                auto properties = VkPhysicalDeviceProperties{};
                vkGetPhysicalDeviceProperties(device, &properties);
                uint32_t queueFamilityCount = 0;

                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilityCount, nullptr);
                std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilityCount);
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilityCount, queueFamilyProperties.data());

                VkBool32 foundGraphics = false;
                VkBool32 foundCompute = false;
                VkBool32 foundTransfer = false;
                VkBool32 foundPresent = false;

                std::vector<uint32_t> graphicsSupportedQueues;
                std::vector<uint32_t> transferSupportedQueues;
                std::vector<uint32_t> computeSupportedQueues;
                std::vector<uint32_t> presentSupportedQueues;

                if (!properties.deviceType & VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                    score  = 0;
                for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
                {
                    if (getQueuFamilieIndex(queueFamilyProperties[i], VK_QUEUE_GRAPHICS_BIT))
                    {
                        graphicsSupportedQueues.push_back(i);
                        foundGraphics = 1;
                    }
                    if (getQueuFamilieIndex(queueFamilyProperties[i], VK_QUEUE_COMPUTE_BIT))
                    {
                        computeSupportedQueues.push_back(i);
                        foundCompute = 1;
                    }
                    if (getQueuFamilieIndex(queueFamilyProperties[i], VK_QUEUE_TRANSFER_BIT))
                    {
                        transferSupportedQueues.push_back(i);
                        foundTransfer = 1;
                    }

                    if (!foundPresent)
                    {
                        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &foundPresent);
                        presentSupportedQueues.push_back(i);
                        foundPresent = 1;
                    }
                }

                std::vector<uint32_t> usedQueues;

                if (graphicsSupportedQueues.size() > 0)
                    usedQueues.push_back(graphicsSupportedQueues[0]);
                auto getUniqueQueueIndex = [](std::vector<uint32_t>& usedQueues, std::vector<uint32_t> supportQueues)
                {
                    for (auto i : usedQueues)
                    {
                        for (auto j : supportQueues)
                        {
                            if (i != j)
                            {
                                usedQueues.push_back(j);
                                return j;
                            }
                        }
                    }
                    return static_cast<uint32_t>(0);
                };
                queueFamilies.graphics = getUniqueQueueIndex(usedQueues, graphicsSupportedQueues);
                queueFamilies.transfer = getUniqueQueueIndex(usedQueues, transferSupportedQueues);
                queueFamilies.compute  = getUniqueQueueIndex(usedQueues, computeSupportedQueues);
                queueFamilies.present  = getUniqueQueueIndex(usedQueues, presentSupportedQueues);

                // check if the device have the required extensions
                if (!checkDeviceExtensions(device))
                    score = 0;
                if (!querySwapChainProperties(device, surface))
                    score = 0; // check if the device is suitable for the swapChain

                bool featuresQuery = features.samplerAnisotropy && features.shaderClipDistance && features.
                    fillModeNonSolid;

                if (!featuresQuery)
                    score = 0;

                return score;
            }

            bool Device::getQueuFamilieIndex(VkQueueFamilyProperties props, VkQueueFlagBits queueFlags)
            {
                if (queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    if (props.queueFlags & queueFlags)
                    {
                        return true;
                    }
                }

                else if (queueFlags & VK_QUEUE_COMPUTE_BIT)
                {
                    if (props.queueFlags & queueFlags)
                        return true;
                }

                else if (queueFlags & VK_QUEUE_TRANSFER_BIT)
                {
                    if (props.queueFlags & VK_QUEUE_TRANSFER_BIT)
                        return true;
                }

                return false;
            }

            bool Device::querySwapChainProperties(VkPhysicalDevice device, Surface surface)
            {
                SwapchainQueryProperties properties;
                uint32_t formatCount;
                uint32_t presentModeCount;
                // vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, *surface, &swapChainExt.capabilities);
                
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
                properties.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, properties.formats.data());

                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
                properties.presentationModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                                          properties.presentationModes.data());
                return  !properties.formats.empty() && !properties.presentationModes.empty();;
            }
            
            const VkAllocationCallbacks* Device::getAllocator()
            {
                return nullptr;
            }

            void Device::createLogicalDevice()
            {
                std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
                std::set<uint32_t> uniqueFamilies = {
                    queueFamilies.graphics.value(), queueFamilies.present.value(), queueFamilies.transfer.value(),
                    queueFamilies.compute.value()
                };

                for (uint32_t queueFamily : uniqueFamilies)
                {
                    VkDeviceQueueCreateInfo QcreateInfo{};
                    QcreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    QcreateInfo.queueCount = 1;
                    QcreateInfo.queueFamilyIndex = queueFamily;
                    float priority = 1.0f;
                    QcreateInfo.pQueuePriorities = &priority;
                    queueCreateInfos.push_back(QcreateInfo);
                }

                VkPhysicalDeviceVulkan12Features features_12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};

                VkPhysicalDeviceFeatures features{};
                features.samplerAnisotropy = VK_TRUE;
                features.fillModeNonSolid = VK_TRUE;

                VkPhysicalDeviceMeshShaderFeaturesEXT mesh_shader_feature{};
                mesh_shader_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
                mesh_shader_feature.meshShader = VK_TRUE;
                mesh_shader_feature.taskShader = VK_TRUE;

                vk_PhysicalDevicefeatures2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,&mesh_shader_feature} ;
                vk_PhysicalDevicefeatures2.features.pipelineStatisticsQuery = true;
                vk_PhysicalDevicefeatures2.features.multiDrawIndirect = true;
                vk_PhysicalDevicefeatures2.features.depthBounds = true;


                //Chain StorageBuffer16BitAccess
                VkPhysicalDeviceVulkan11Features features11 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
                features11.storageBuffer16BitAccess = true;
                features11.shaderDrawParameters = true;
                mesh_shader_feature.pNext = &features11;

                //Chain Vulkan_1_2 Features
                VkPhysicalDeviceVulkan12Features features12 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
                features12.drawIndirectCount = true;
                features12.storageBuffer8BitAccess = true;
                features12.uniformAndStorageBuffer8BitAccess = true;
                features12.shaderFloat16 = true;
                features12.shaderInt8 = true;
                features12.samplerFilterMinmax = true;
                features12.scalarBlockLayout = true;
                features11.pNext = &features12;

                VkPhysicalDeviceFeatures deviceFeatures{};
                deviceFeatures.pipelineStatisticsQuery = VK_TRUE;

                VkDeviceCreateInfo deviceInfo = {};
                deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
                deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
                deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
                deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
                deviceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                deviceInfo.ppEnabledLayerNames = validationLayers.data();
                deviceInfo.pNext = &vk_PhysicalDevicefeatures2;

                VkResult result = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &logicalDevice);

                if (result != VK_SUCCESS)
                    Logger::Error("Failed to find a suitable device");

                if (result == VK_SUCCESS)
                {
                    Logger::Debug("Successfully created a logical device");
                    queueFamilies.isComplete();
                }
                vkGetDeviceQueue(logicalDevice, queueFamilies.graphics.value(), 0, &graphicsQueue);
                vkGetDeviceQueue(logicalDevice, queueFamilies.present.value(), 0, &presentationQueue);
                vkGetDeviceQueue(logicalDevice, queueFamilies.transfer.value(), 0, &transferQueue);
                vkGetDeviceQueue(logicalDevice, queueFamilies.compute.value(), 0, &computeQueue);

                depth_format = get_depth_supported_format({ VK_FORMAT_D32_SFLOAT,
                                                     VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                     VK_FORMAT_D24_UNORM_S8_UINT },
                                                     VK_IMAGE_TILING_OPTIMAL,
                                                     VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
            }

            size_t Device::getMinimumBufferAligment()
            {
                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(physicalDevice, &props);
                return props.limits.minUniformBufferOffsetAlignment;
            }

            QueueSharingMode Device::getQueueSharingMode()
            {
                QueueSharingMode queue_sharing{};
                if (queueFamilies.graphics.value() != queueFamilies.present.value())
                {
                    queue_sharing.sharingMode = VK_SHARING_MODE_CONCURRENT;
                    queue_sharing.queueFamiliyIndexCount = 2;
                    std::vector<uint32_t> queueF(2);
                    queueF = {queueFamilies.graphics.value(), queueFamilies.present.value()};
                    queue_sharing.queueFamilies = queueF.data();
                    return queue_sharing;
                }

                queue_sharing.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                queue_sharing.queueFamiliyIndexCount = 1;
                queue_sharing.queueFamilies = &queueFamilies.graphics.value();
                return queue_sharing;
            }

            void Device::destroyDevice()
            {
                vkDestroyDevice(logicalDevice, nullptr);
                Logger::Debug("SuccessFully destroyed Device");
            }


            VkDevice& Device::getLogicalDevice()
            {
                return logicalDevice;
            }

            VkDevice* Device::getLogicalDevicePtr()
            {
                return &logicalDevice;
            }

            VkPhysicalDevice& Device::getPhysicalDevice()
            {
                return physicalDevice;
            }

            VkPhysicalDevice* Device::getPhysicalDevicePtr()
            {
                return &physicalDevice;
            }

            bool Device::prepareDevice(VkSurfaceKHR surface, Instance instance)
            {
                choosePhysicalDevice(instance, surface);
                createLogicalDevice();
                return true;
            }

            uint32_t Device::getGraphicsQueueIndex() const
            {
                return queueFamilies.graphics.value();
            }

            uint32_t Device::getTransferQueueIndex() const
            {
                return queueFamilies.transfer.value();
            }

            uint32_t Device::getComputeQueueIndex() const
            {
                return queueFamilies.compute.value();
            }

            uint32_t Device::getPresentQueueIndex() const
            {
                return queueFamilies.present.value();
            }


            VkQueue Device::getGraphicsQueueHandle() const
            {
                return graphicsQueue;
            }

            VkQueue Device::getTransferQueueHandle() const
            {
                return transferQueue;
            }

             VkQueue Device::getComputeQueueHandle() const
            {
                return computeQueue;
            }

            VkFormat
                Device::get_depth_supported_format(const std::vector<VkFormat>&candidates,
                                               VkImageTiling tiling,
                                               VkFormatFeatureFlags features) {
                for (VkFormat format : candidates) {
                    VkFormatProperties props;
                    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

                    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                        return format;
                    }
                    else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features){
                        return format;
                    }
                }

                throw std::runtime_error("failed to find supported format!");
            }

            VkFormat Device::get_depth_format() {
                return depth_format;
            }
        }
    }
}
