#ifndef IMAGE_
#define IMAGE_
#include "Device/Device.h"
namespace RenderSystem
{
    namespace VulkanBackend
    {
        class Image{

            uint32_t numLayers;
			VkExtent2D vk_extent;
			const Device* device;
			VkImage vk_image;
			VkImageView vk_view;
			VkDeviceMemory vk_deviceMemory;

	        Image(Device* device, uint32_t Width, uint32_t Height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageCreateFlags flags, VkImageAspectFlags aspectFlags, uint32_t arrayLayers = 1, bool useMaxNumMips = 0, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

			Image(Device* device, uint32_t Width, uint32_t Height, VkImage& image, VkImageView& view);
		//	Image(const Device* device, const char* path, VkFormat format, VkImageTiling tiling, VkImageCreateFlags flags, VK_Objects::CommandPool& pool, uint32_t arrayLayers=1, bool useMaxNumMips=0);
		
			const VkImageView* getVkImageViewHandle();

			VkImage& getVkImageHandle();

			VkExtent2D getExtent();

			uint32_t getNumberOfLayers();

			void destroy();

			uint32_t getMaximumMips();

			~Image();
        };

    }

}

#endif