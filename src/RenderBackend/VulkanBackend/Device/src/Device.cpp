#include "Device/Device.h"
#include "Logger/Logger.hpp"

namespace Canella
{
	namespace RenderSystem
	{
		namespace VulkanBackend
		{
			Device::Device() {};

			void Device::choosePhysicalDevice(Instance instance, Surface surface)
			{

				uint32_t deviceCount = 0;
				vkEnumeratePhysicalDevices(instance.handle, &deviceCount, nullptr);

				if (deviceCount == 0)
				{
					Canella::Logger::Error("Failed to find a proper Graphics Card");
				};

				std::vector<VkPhysicalDevice> devices(deviceCount);
				vkEnumeratePhysicalDevices(instance.handle, &deviceCount, devices.data());

				// Score of the GPU
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

					currentScore = scorePhysicalDevice(device, features, props, surface);
					// currentScore >=100. means that the device have all the minimum demanded capabilities
					if (currentScore >= 100.)
					{
						findSuitableGPU = true;
						bestDevice = device;
						bestScore = currentScore;
					}
					// If we find any better device we pick it
					if (currentScore > bestScore)
					{
						bestDevice = device;
						bestScore = currentScore;
					}
				}

				if (!findSuitableGPU)
				{

					Canella::Logger::Error("Failed to find a  Graphics Card that suits the requirements");
					return;
				}

				else
				{

					physicalDevice = bestDevice;
					// Query physical device properties
					vkGetPhysicalDeviceProperties(physicalDevice, &vk_physicalDeviceProperties);
					vkGetPhysicalDeviceMemoryProperties(physicalDevice, &vk_MemoryProperties);
					vkGetPhysicalDeviceFeatures(physicalDevice, &vk_PhysicalDevicefeatures);

					Canella::Logger::Info("Device %c ", vk_physicalDeviceProperties.deviceName);

					physicalDevice = bestDevice;
					msaaSamples = getMaxUsableSampleCount();
					Canella::Logger::Info("MaxSamples Count %d ", msaaSamples);
				}
			}

			bool Device::checkDeviceExtensions(VkPhysicalDevice device)
			{
				uint32_t extensionsCount;
				vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

				std::vector<VkExtensionProperties> extensions(extensionsCount);
				vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensions.data());

				std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

				for (const auto& extension : extensions)
				{
					requiredExtensions.erase(extension.extensionName);
				}

				return requiredExtensions.empty();
			}
			VkSampleCountFlagBits Device::getMaxUsableSampleCount()
			{
				VkPhysicalDeviceProperties physicalDeviceProperties;
				vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

				VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
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

			int Device::scorePhysicalDevice(VkPhysicalDevice device, VkPhysicalDeviceFeatures features, VkPhysicalDeviceMemoryProperties memProperties, Surface surface)
			{

				/*this is not really good yet ,
				But for now It just take a suitable device with the largest heap
				*/
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

				// If we can't find unique index for each queue, we use the back index
	
				std::vector<uint32_t> graphicsSupportedQueues;
				std::vector<uint32_t> transferSupportedQueues;
				std::vector<uint32_t> computeSupportedQueues;
				std::vector<uint32_t> presentSupportedQueues;


				auto unique_queues = [](uint32_t graphics, uint32_t transfer, uint32_t compute, uint32_t present) {
				};

				if (!properties.deviceType & VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					score -= 100;
				}

				

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

				if (graphicsSupportedQueues.size() > 0) usedQueues.push_back(graphicsSupportedQueues[0]);
				auto getUniqueQueueIndex = [](std::vector<uint32_t>& usedQueues, std::vector<uint32_t> supportQueues) {
					for (auto i : usedQueues) {
						for (auto j : supportQueues) {
							if (i != j) {
								usedQueues.push_back(j);
								return j;
							}
						}
					}
					return static_cast<uint32_t>(0);
				};
				queueFamilies.graphics = getUniqueQueueIndex(usedQueues, graphicsSupportedQueues);
				queueFamilies.transfer = getUniqueQueueIndex(usedQueues, transferSupportedQueues);
				queueFamilies.compute = getUniqueQueueIndex(usedQueues, computeSupportedQueues);
				queueFamilies.present = getUniqueQueueIndex(usedQueues, presentSupportedQueues);

				// if device has no graphics neither presentation capabilities, the device is not suitable (score = 0)
				if (!foundGraphics)
					score = 0;
				if (!foundPresent)
					score = 0;
				if (!foundTransfer)
					score = 0; // means that the device doesn't have dedicated queue to transfer operations
				if (!foundCompute)
					score -= 40; // means that the device doesn't have dedicated queue to compute operations

				if (!checkDeviceExtensions(device))
					score = 0; // check if the device have the required extensions
				if (!querySwapChainProperties(device, surface))
					score = 0; // check if the device is suitable for the swapChain

				if (memProperties.memoryHeaps->size > 2000)
					score += 10;
				if (memProperties.memoryHeaps->size > 4000)
					score += 10;
				if (memProperties.memoryHeaps->size > 8000)
					score += 10;

				bool featuresQuery = features.samplerAnisotropy && features.shaderClipDistance && features.fillModeNonSolid;

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
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, properties.presentationModes.data());

				bool isSuitableSwapChain = false;
				isSuitableSwapChain = !properties.formats.empty() && !properties.presentationModes.empty();

				return isSuitableSwapChain;
			}
			void Device::createLogicalDevice()
			{
				std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
				std::set<uint32_t> uniqueFamilies = { queueFamilies.graphics.value(), queueFamilies.present.value(), queueFamilies.transfer.value(), queueFamilies.compute.value() };

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

				VkPhysicalDeviceFeatures features{};
				features.samplerAnisotropy = VK_TRUE;
				features.fillModeNonSolid = VK_TRUE;

				VkDeviceCreateInfo deviceInfo = {};
				deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
				deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
				deviceInfo.pEnabledFeatures = &features;
				deviceInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
				deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
				deviceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
				deviceInfo.ppEnabledLayerNames = validationLayers.data();

				VkResult result = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &logicalDevice);

				if (result != VK_SUCCESS)
				{
					Canella::Logger::Error("Failed to find a suitable device");
				}

				if (result == VK_SUCCESS)
				{
					Canella::Logger::Debug("Successfully created a logical device");
					queueFamilies.isComplete();
				}
				vkGetDeviceQueue(logicalDevice, queueFamilies.graphics.value(), 0, &graphicsQueue);
				vkGetDeviceQueue(logicalDevice, queueFamilies.present.value(), 0, &presentationQueue);
				vkGetDeviceQueue(logicalDevice, queueFamilies.transfer.value(), 0, &transferQueue);
				vkGetDeviceQueue(logicalDevice, queueFamilies.compute.value(), 0, &computeQueue);
			}
			size_t Device::getMinimumBufferAligment()
			{
				VkPhysicalDeviceProperties props;
				vkGetPhysicalDeviceProperties(physicalDevice, &props);
				return props.limits.minUniformBufferOffsetAlignment;
			}
			QueueSharingMode Device::getQueueSharingMode()
			{
				uint32_t indexCount = 1;
	
					
				QueueSharingMode queueSharing;
				if (queueFamilies.graphics.value() != queueFamilies.present.value())
				{

					queueSharing.sharingMode = VK_SHARING_MODE_CONCURRENT;
					queueSharing.queueFamiliyIndexCount = 2;
					std::vector<uint32_t> queueF(2);
					queueF = { queueFamilies.graphics.value(), queueFamilies.present.value() };
					queueSharing.queueFamilies = queueF.data();
					return queueSharing;
				}

				queueSharing.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				queueSharing.queueFamiliyIndexCount = 1;
				queueSharing.queueFamilies = &queueFamilies.graphics.value();
				return queueSharing;
			}
			void Device::destroyDevice()
			{
				vkDestroyDevice(logicalDevice, nullptr);
				Canella::Logger::Debug("SuccessFully destroyed Device");
			}
			Device::~Device()
			{
				destroyDevice();
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
		}
	}
}