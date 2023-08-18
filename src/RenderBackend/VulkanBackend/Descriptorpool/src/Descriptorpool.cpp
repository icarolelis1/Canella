#include "Descriptorpool/Descriptorpool.h"
#include "CanellaUtility/CanellaUtility.h"
#include <json.hpp>

using namespace Canella::RenderSystem::VulkanBackend;

void Descriptorpool::build_descriptor_pool(Device* device)
{
    build_global_descriptor_pool(*device);
    build_bindless_descriptor_pool(*device);
}
//TODO REBUILD THE DESCRIPTOR WHEN REACHIN MAXIMUM AMMOUNT OF ALOCATIONS    
void Descriptorpool::build_global_descriptor_pool(Device& device)
{
    static constexpr uint32_t k_global_pool_elements = 612;
    constexpr VkDescriptorPoolSize pool_sizes[] =
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
    pool_info.maxSets = k_global_pool_elements * static_cast<uint32_t>(std::size(pool_sizes));
    pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
    pool_info.pPoolSizes = pool_sizes;

    if ((vkCreateDescriptorPool(device.getLogicalDevice(),
                                &pool_info,
                                device.getAllocator(),
                                &vk_global_descriptorpool) != VK_SUCCESS))
        Logger::Error("Failed to create global DescriptorPool");
}

void Descriptorpool::build_bindless_descriptor_pool(Device& device)
{
    const VkDescriptorPoolSize pool_sizes_bindless[] =
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
    constexpr uint32_t pool_count = std::size(pool_sizes_bindless);

    VkDescriptorPoolCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    create_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    create_info.maxSets = k_max_bindless_resources * static_cast<uint32_t>(pool_count);
    create_info.poolSizeCount = static_cast<uint32_t>(pool_count);
    create_info.pPoolSizes = pool_sizes_bindless;
    if (const VkResult result = vkCreateDescriptorPool(device.getLogicalDevice(), &create_info, device.getAllocator(),
                                                       &vk_bindless_descriptorpool); result != VK_SUCCESS)
        Logger::Error("failed to create bindless DescriptorPool");
}

void Descriptorpool::build_descriptor_set_layout(Device& device)
{
    constexpr uint32_t pool_count = 2;
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


void Descriptorpool::allocate_descriptor_set(Device& device, std::shared_ptr<DescriptorSetLayout> layout,
                                           VkDescriptorSet& set)
{
    VkDescriptorSetAllocateInfo allocInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocInfo.descriptorPool = vk_global_descriptorpool;
    allocInfo.descriptorSetCount =1;
    allocInfo.pSetLayouts = &layout->getDescriptorLayoutHandle();
    if (const auto result = vkAllocateDescriptorSets(device.getLogicalDevice(),
                                                            &allocInfo,
                                                           &set);
    result != VK_SUCCESS)
        Logger::Debug("Failed to Allocated DescriptorSet");
}

void Descriptorpool::build(Device* _device)
{
    device = _device;
    build_descriptor_pool(device);
}

void Descriptorpool::destroy() {
    vkDestroyDescriptorPool(device->getLogicalDevice(),vk_bindless_descriptorpool,device->getAllocator());
    vkDestroyDescriptorPool(device->getLogicalDevice(),vk_global_descriptorpool,device->getAllocator());
}

void Descriptorpool::free_descriptorsets(Device &device, const VkDescriptorSet *sets,uint32_t count) {

    VK_CHECK(vkFreeDescriptorSets(device.getLogicalDevice(),
                                  vk_global_descriptorpool,
                                  count,
                                  sets),
                                  "Failed to free descriptorsets");
}
