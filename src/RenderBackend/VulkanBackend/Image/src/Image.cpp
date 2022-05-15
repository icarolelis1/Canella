// #include "Image/Image.h"

// namespace RenderSystem
// {
//     namespace VulkanBackend
//     {

//     Image::Image(Device* device, uint32_t Width, uint32_t Height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageCreateFlags flags, VkImageAspectFlags aspectFlags, uint32_t arrayLayers = 1, bool useMaxNumMips = 0, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT)
//     {

// 	uint32_t numMips = 1;

// 	if (useMaxNumMips) {

// 		numMips = getMaximumMips();
// 	}
// 	vk_extent.width = Width;
// 	vk_extent.height = Height;

// 	VkImageCreateInfo imageInfo = {};
// 	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
// 	imageInfo.imageType = VK_IMAGE_TYPE_2D;
// 	imageInfo.extent.width = Width;
// 	imageInfo.extent.height = Height;
// 	imageInfo.extent.depth = 1;
// 	imageInfo.mipLevels = numMips;
// 	imageInfo.arrayLayers = arrayLayers;
// 	imageInfo.format = format;
// 	imageInfo.tiling = tiling;
// 	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 	imageInfo.usage = usage;
// 	imageInfo.samples = samples;
// 	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
// 	imageInfo.flags = flags;

// 	if (vkCreateImage(device->getLogicalDevice(), &imageInfo, nullptr, &vk_image) != VK_SUCCESS) {
// 		throw std::runtime_error("    Failed to create image\n");
// 	}

// 	VkMemoryRequirements memRequirements;
// 	vkGetImageMemoryRequirements(device->getLogicalDevice(), vk_image, &memRequirements);

// 	VkMemoryAllocateInfo allocInfo = {};
// 	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
// 	allocInfo.allocationSize = memRequirements.size;
// 	//allocInfo.memoryTypeIndex = :findMemoryType(device->getPhysicalDevice(), memRequirements.memoryTypeBits, properties);

// 	if (vkAllocateMemory(device->getLogicalDevice(), &allocInfo, nullptr, &vk_deviceMemory) != VK_SUCCESS) {
// 		throw std::runtime_error("    Failed to allocate image memory!\n");
// 	}

// 	vkBindImageMemory(device->getLogicalDevice(), vk_image, vk_deviceMemory, 0);

// 	//Image View Creation
// 	VkImageViewCreateInfo viewInfo = {};
// 	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
// 	viewInfo.image = vk_image;
// 	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
// 	viewInfo.format = format;
// 	viewInfo.subresourceRange.aspectMask = aspectFlags;
// 	viewInfo.subresourceRange.baseMipLevel = 0;
// 	viewInfo.subresourceRange.levelCount = numMips;
// 	viewInfo.subresourceRange.baseArrayLayer = 0;
// 	viewInfo.subresourceRange.layerCount = 1;
// 	VkResult result;
// 	result = vkCreateImageView(device->getLogicalDevice(), &viewInfo, nullptr, &vk_view);

// 	//if (result != VK_SUCCESS)Utils::LOG("Failed to create imageView\n");

// }

// Image::Image(const Device* device, uint32_t Width, uint32_t Height, VkImage& image, VkImageView& view)
// {
// 	vk_image = image;
// 	vk_view = view;
// }


// VK_Objects::Image::Image(const VK_Objects::Device* _device, const char* path, VkFormat format, VkImageTiling tiling, VkImageCreateFlags flags, VK_Objects::CommandPool&  pool, uint32_t arrayLayers, bool useMaxNumMips):device(_device)
// {

// 	uint32_t numMips = 1;

	

// 	int texWidth, texHeight, texChannels;
// 	stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
// 	vk_extent.width = texWidth;
// 	vk_extent.height = texHeight;

// 	const uint32_t maxMips = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

// 	if (useMaxNumMips) {

// 		numMips = maxMips < 10 ? maxMips : 10;
// 	}


// 	VkDeviceSize imageSize = texWidth * texHeight * 4;
// 	if (!pixels) {
// 		std::cout << path << std::endl;
// 		throw std::runtime_error("failed to load texture image for path : ~");
// 	}

// 	VkImageCreateInfo imageInfo = {};
// 	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
// 	imageInfo.imageType = VK_IMAGE_TYPE_2D;
// 	imageInfo.extent.width = texWidth;
// 	imageInfo.extent.height = texHeight;
// 	imageInfo.extent.depth = 1;
// 	imageInfo.mipLevels = numMips;
// 	imageInfo.arrayLayers = arrayLayers;
// 	imageInfo.format = format;
// 	imageInfo.tiling = tiling;
// 	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
// 	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
// 	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
// 	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
// 	imageInfo.flags = flags;

// 	if (vkCreateImage(device->getLogicalDevice(), &imageInfo, device->getAllocator(), &vk_image) != VK_SUCCESS) {
// 		throw std::runtime_error("    Failed to create image\n");
// 	}

// 	VkMemoryRequirements memRequirements;
// 	vkGetImageMemoryRequirements(device->getLogicalDevice(), vk_image, &memRequirements);

// 	VkMemoryAllocateInfo allocInfo = {};
// 	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
// 	allocInfo.allocationSize = memRequirements.size;
// 	//allocInfo.memoryTypeIndex = Vk_Functions::findMemoryType(device->getPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

// 	if (vkAllocateMemory(device->getLogicalDevice(), &allocInfo, nullptr, &vk_deviceMemory) != VK_SUCCESS) {
// 		throw std::runtime_error("    Failed to allocate image memory!\n");
// 	}

// 	vkBindImageMemory(device->getLogicalDevice(), vk_image, vk_deviceMemory, 0);

// 	//Image View Creation
// 	VkImageViewCreateInfo viewInfo = {};
// 	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
// 	viewInfo.image = vk_image;
// 	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
// 	viewInfo.format = format;
// 	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 	viewInfo.subresourceRange.baseMipLevel = 0;
// 	viewInfo.subresourceRange.levelCount = numMips;
// 	viewInfo.subresourceRange.baseArrayLayer = 0;
// 	viewInfo.subresourceRange.layerCount = 1;
// 	VkResult result;
// 	result = vkCreateImageView(device->getLogicalDevice(), &viewInfo, nullptr, &vk_view);

// 	if (result != VK_SUCCESS)Utils::LOG("Failed to create imageView\n");


// 	VK_Objects::Buffer stagingBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
// 	stagingBuffer.id = "text";
// 	void* data;
// 	vkMapMemory(device->getLogicalDevice(), stagingBuffer.getMemoryHandle(), 0, imageSize, 0, &data);
// 	memcpy(data, pixels, static_cast<size_t>(imageSize));
// 	vkUnmapMemory(device->getLogicalDevice(), stagingBuffer.getMemoryHandle());

// 	free(pixels);
// 	Vk_Functions::setImageLayout(*device, pool, vk_image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, 0, numMips);
// 	Vk_Functions::copyBufferToImage(stagingBuffer, vk_image, *device, pool, texWidth, texHeight, 0, 1);
// 	Vk_Functions::setImageLayout(*device, pool, vk_image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, 0, numMips);

// }

// const VkImageView* VK_Objects::Image::getVkImageViewHandle()
// {
// 	return &vk_view;
// }

// VkImage& VK_Objects::Image::getVkImageHandle()
// {

// 	return vk_image;
// }




// VkExtent2D VK_Objects::Image::getExtent()
// {
// 	return vk_extent;
// }


// uint32_t VK_Objects::Image::getNumberOfLayers()
// {
// 	return numLayers;
// }

// void VK_Objects::Image::destroy()
// {
// 	vkDestroyImage(device->getLogicalDevice(), vk_image, device->getAllocator());
// 	vkDestroyImageView(device->getLogicalDevice(), vk_view, device->getAllocator());
// 	vkFreeMemory(device->getLogicalDevice(), vk_deviceMemory, device->getAllocator());
// }

// VK_Objects::Image::~Image()
// {
// 	destroy();
// }

// uint32_t VK_Objects::Image::getMaximumMips()
// {
// 	uint32_t maxMips =  static_cast<uint32_t>(std::floor(std::log2(std::max(vk_extent.width, vk_extent.height)))) + 1;
// 	 maxMips =  maxMips < 10 ? maxMips : 10;
// 	 return maxMips;

// }

//     }

// }