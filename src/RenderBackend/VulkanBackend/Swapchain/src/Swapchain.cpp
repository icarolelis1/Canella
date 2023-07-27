#include "Swapchain/Swapchain.h"
#include <Logger/Logger.hpp>

namespace Canella
{
	namespace RenderSystem
	{
		namespace VulkanBackend
		{
			bool Swapchain::query_swapchain_properties(VkPhysicalDevice device, VkSurfaceKHR surface, VkFormat &desiredFormat, GLFWwindow *window)
			{

				SwapchainQueryProperties propertiesQuery{};

				uint32_t formatCount;
				uint32_t presentationCount;
				// Surface Cabalities
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &properties.capabilities);
				// SUrface supported formats
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
				propertiesQuery.formats.resize(formatCount);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, propertiesQuery.formats.data());
				// Surface supported Presentation Modes
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);
				propertiesQuery.presentationModes.resize(presentationCount);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, propertiesQuery.presentationModes.data());

				properties.format = chooseFormat(propertiesQuery.formats, desiredFormat);
				properties.extent = chooseImageExtent(properties.capabilities, window);
				properties.presentMode = choosePresentationMode(propertiesQuery.presentationModes);
				properties.minImageCount =  properties.capabilities.minImageCount +1;

				if (properties.capabilities.maxImageCount > 0 && properties.minImageCount > properties.capabilities.maxImageCount)
					properties.minImageCount = properties.capabilities.maxImageCount;

				bool isSuitableSwapChain = false;
				isSuitableSwapChain = !propertiesQuery.formats.empty() && !propertiesQuery.presentationModes.empty();
				return isSuitableSwapChain;
			}

			void Swapchain::prepare_swapchain(uint32_t WIDTH,
                                              uint32_t HEIGHT,
                                              Device &device,
                                              VkSurfaceKHR surface,
                                              ImageFormat desiredFormat,
                                              GLFWwindow *window,
                                              QueueSharingMode queueSharingMode)
			{
				width = WIDTH;
				height = HEIGHT;

                query_swapchain_properties(device.getPhysicalDevice(), surface, desiredFormat, window);
                create_swapchain(device, surface, queueSharingMode);
                aquire_swapchain_images(device);
                create_swapchain_views(device);
			}

			VkFormat Swapchain::getFormat() { return properties.format.format; }

			VkExtent2D Swapchain::getExtent()

			{
				return properties.extent;
			}

			/**
			 * \brief Gets a VkViewport with dimensions of swapchain framebuffer
			 * \return VkViewport with dimensions of swapchain framebuffer
			 */
			VkViewport Swapchain::get_view_port()
			{
				VkViewport vk_viewport{};
                vk_viewport.x = 0;
				vk_viewport.width = properties.extent.width;
				vk_viewport.height = properties.extent.height;
                vk_viewport.minDepth = 0;
				vk_viewport.maxDepth = 1;
 				return vk_viewport;
			}

			VkRect2D Swapchain::get_rect2d()
			{
				VkRect2D rect;
				rect.extent.width = properties.extent.width;
				rect.extent.height = properties.extent.height;
				rect.offset = {0, 0};
				return rect;
			}

			VkSwapchainKHR &Swapchain::get_swap_chain_handle()
			{
				return properties.vk_swapChain;
			}

			int8_t Swapchain::get_number_of_images()
			{
				return static_cast<uint32_t>(vk_images.size());
			}
			VkExtent2D Swapchain::chooseImageExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window)
			{

				if (capabilities.currentExtent.width != UINT32_MAX)
				{
					return capabilities.currentExtent;
				}
				else
				{
					int w, h;
					VkExtent2D actualExtent;
					//	= { WIDTH, HEIGHT };
					glfwGetFramebufferSize(window, &w, &h);
					actualExtent.width = static_cast<uint32_t>(w);
					actualExtent.height = static_cast<uint32_t>(h);

					return actualExtent;
				}
			}

			VkPresentModeKHR Swapchain::choosePresentationMode(std::vector<VkPresentModeKHR> &presentModes)
			{
				for (const auto &presentMode : presentModes)
				{
					if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
					{
						Canella::Logger::Info("PRESENTATION MODE : VK_PRESENT_MODE_MAILBOX_KHR");
						return presentMode;
					}
				}
				Canella::Logger::Info("PRESENTATION MODE : VK_PRESENT_MODE_FIFO_KHR");

				// If we can't get MailBox we go with FIFO
				return VK_PRESENT_MODE_FIFO_KHR;
			}

			VkSurfaceFormatKHR Swapchain::chooseFormat(std::vector<VkSurfaceFormatKHR> formats, VkFormat desiredFormat)
			{
				// We try to get the desiredFormat
				for (const auto &format : formats)
				{
					if (format.format == desiredFormat && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
						return format;
				}
				// If we don't get the first format, we take the first one.
				return formats[0];
			}
			void Swapchain::create_swapchain_views(Device &device)
			{

				size_t n = vk_images.size();
				vk_imageViews.resize(n);

				for (size_t i = 0; i < n; i++)
				{

					VkImageViewCreateInfo viewInfo = {};
					viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					viewInfo.image = vk_images[i];
					viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
					viewInfo.format = properties.format.format;
					viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					viewInfo.subresourceRange.baseMipLevel = 0;
					viewInfo.subresourceRange.levelCount = 1;
					viewInfo.subresourceRange.baseArrayLayer = 0;
					viewInfo.subresourceRange.layerCount = 1;
					VkResult result;
					result = vkCreateImageView(device.getLogicalDevice(), &viewInfo, nullptr, &vk_imageViews[i]);
					if (result != VK_SUCCESS)
					{
						Canella::Logger::Error("Failed to create Swapchain Images");
						return;
					}
				}
				Canella::Logger::Debug("Sucessfully created Swapchain Images");
			}

			void Swapchain::aquire_swapchain_images(Device &device)
			{

				uint32_t imageCount;
				vkGetSwapchainImagesKHR(device.getLogicalDevice(), properties.vk_swapChain, &imageCount, nullptr);
				vk_images.resize(imageCount);
				vkGetSwapchainImagesKHR(device.getLogicalDevice(), properties.vk_swapChain, &imageCount, vk_images.data());
			}

			void Swapchain::create_swapchain(Device &device, Surface &surface, QueueSharingMode &queueSharingMode)
			{

				VkSwapchainCreateInfoKHR createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				createInfo.surface = surface;
				createInfo.clipped = VK_TRUE;
				createInfo.imageFormat = properties.format.format;
				createInfo.minImageCount = properties.minImageCount;
				createInfo.imageArrayLayers = 1;
				createInfo.imageColorSpace = properties.format.colorSpace;
				createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				createInfo.presentMode = properties.presentMode;
				createInfo.imageExtent = properties.extent;
				createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
				createInfo.preTransform = properties.capabilities.currentTransform;
                createInfo.preTransform =VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

				if (queueSharingMode.queueFamiliyIndexCount > 1)
				{
					createInfo.imageSharingMode = queueSharingMode.sharingMode;
					createInfo.queueFamilyIndexCount = queueSharingMode.queueFamiliyIndexCount;
				}

				else
				{
					createInfo.imageSharingMode = queueSharingMode.sharingMode;
					createInfo.queueFamilyIndexCount = queueSharingMode.queueFamiliyIndexCount;
				}

				createInfo.pQueueFamilyIndices = queueSharingMode.queueFamilies;

				createInfo.oldSwapchain = VK_NULL_HANDLE;
				createInfo.pNext = VK_NULL_HANDLE;
				VkResult result = vkCreateSwapchainKHR(device.getLogicalDevice(), &createInfo, nullptr, &properties.vk_swapChain);

				if (result == VK_SUCCESS)
					Canella::Logger::Info("Successfully created swapchain");
				else
				{
					Canella::Logger::Error("failed to create swapchain");
					return;
				}
			}

			void Swapchain::destroy_swapchain(Device &device)
			{
				for (auto view : vk_imageViews)
				{
					vkDestroyImageView(device.getLogicalDevice(), view, nullptr);
					vkDestroySwapchainKHR(device.getLogicalDevice(), properties.vk_swapChain, nullptr);
					Canella::Logger::Info("Successfully destroyed SwapChain");
				}
			}

			std::vector<VkImageView> &VulkanBackend::Swapchain::getViews()
			{
				return vk_imageViews;
			}

            std::vector<VkImage> &Swapchain::get_images() {
                return vk_images;
            }
        }
	}
}