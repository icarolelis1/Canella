#include "Descriptorpool/Descriptorpool.h"

#include <json.hpp>

using namespace Canella::RenderSystem::VulkanBackend;

void Descriptorpool::buildDescriptorPool(Device& device)
{
    buildGlobalDescriptorPool(device);
    buildBindlessDescriptorPool(device);
}

void Descriptorpool::buildGlobalDescriptorPool(Device& device)
{
    static const uint32_t k_global_pool_elements = 128;
    VkDescriptorPoolSize pool_sizes[] =
    {
        {VK_DESCRIPTOR_TYPE_SAMPLER, k_global_pool_elements},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, k_global_pool_elements},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, k_global_pool_elements},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, k_global_pool_elements},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, k_global_pool_elements},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, k_global_pool_elements},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, k_global_pool_elements},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, k_global_pool_elements},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, k_global_pool_elements},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, k_global_pool_elements},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, k_global_pool_elements}
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = k_global_pool_elements * static_cast<uint32_t>(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
    pool_info.poolSizeCount = static_cast<uint32_t>(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
    pool_info.pPoolSizes = pool_sizes;

    if ((vkCreateDescriptorPool(device.getLogicalDevice(), &pool_info, device.getAllocator(), &vk_global_descriptorpool)
        != VK_SUCCESS))
        Logger::Error("Failed to create global DescriptorPool");
}

void Descriptorpool::buildBindlessDescriptorPool(Device& device)
{
    VkDescriptorPoolSize pool_sizes_bindless[] =
    {
        {
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            k_max_bindless_resources
        },
        {
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            k_max_bindless_resources
        },
    };
    const uint32_t pool_count = sizeof(pool_sizes_bindless) / sizeof(pool_sizes_bindless[0]);

    VkDescriptorPoolCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    create_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    create_info.maxSets = k_max_bindless_resources * static_cast<uint32_t>(pool_count);
    create_info.poolSizeCount = static_cast<uint32_t>(pool_count);
    create_info.pPoolSizes = pool_sizes_bindless;
    VkResult result = vkCreateDescriptorPool(device.getLogicalDevice(), &create_info, device.getAllocator(),
                                             &vk_bindless_descriptorpool);
    if (result != VK_SUCCESS)
        Logger::Error("failed to create bindless DescriptorPool");
}

void Descriptorpool::buildDescriptorSetLayout(Device& device)
{
    const uint32_t pool_count = 2;
    VkDescriptorSetLayoutBinding vk_binding[2];
    VkDescriptorSetLayoutBinding& image_sampler_binding = vk_binding[0];
    image_sampler_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    image_sampler_binding.descriptorCount = k_max_bindless_resources;
    image_sampler_binding.binding = k_bindless_texture_binding;

    VkDescriptorSetLayoutBinding& storage_image_binding = vk_binding[1];
    storage_image_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storage_image_binding.descriptorCount = k_max_bindless_resources;
    storage_image_binding.binding = k_bindless_texture_binding + 1;

    VkDescriptorSetLayoutCreateInfo layout_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layout_info.bindingCount = pool_count;
    layout_info.pBindings = vk_binding;
    layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

    VkDescriptorBindingFlags bindless_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
        /*VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |*/ VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
    VkDescriptorBindingFlags binding_flags[4];

    binding_flags[0] = bindless_flags;
    binding_flags[1] = bindless_flags;

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT, nullptr
    };
    extended_info.bindingCount = pool_count;
    extended_info.pBindingFlags = binding_flags;
    layout_info.pNext = &extended_info;

    vkCreateDescriptorSetLayout(device.getLogicalDevice(), &layout_info, device.getAllocator(), &vk_bindless_layout);

    VkDescriptorSetAllocateInfo alloc_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    alloc_info.descriptorPool = vk_bindless_descriptorpool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &vk_bindless_layout;
    vkAllocateDescriptorSets(device.getLogicalDevice(), &alloc_info, &vk_bindless_descriptor_set);
}

void Descriptorpool::AllocateDescriptorSet(Device& device, std::shared_ptr<DescriptorSetLayout> layout,
                                           VkDescriptorSet& set)
{
    VkDescriptorSetAllocateInfo allocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = vk_global_descriptorpool;
    allocInfo.descriptorSetCount =1;
    allocInfo.pSetLayouts = &layout->getDescriptorLayoutHandle();
    auto result = vkAllocateDescriptorSets(device.getLogicalDevice(), &allocInfo, &set);
    if (result != VK_SUCCESS)
        Logger::Debug("Failed to Allocated DescriptorSet");
    Logger::Debug("Allocated DescriptorSet");
}

void Descriptorpool::build(Device& device)
{
    buildDescriptorPool(device);
}
