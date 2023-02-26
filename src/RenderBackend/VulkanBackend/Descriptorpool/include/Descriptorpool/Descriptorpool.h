#ifndef DESCRIPTOR_POOL
#define DESCRIPTOR_POOl
#include "Device/Device.h"
#include "vulkan/vulkan.h"
#include <Pipeline/Pipeline.h>
#include <vector>
namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            class Descriptorpool
            {
            private:
                const uint32_t k_bindless_texture_binding = 10;
                const uint32_t k_max_bindless_resources = 1024;
                VkDescriptorPool vk_bindless_descriptorpool;
                VkDescriptorPool vk_global_descriptorpool;
                VkDescriptorSetLayout vk_bindless_layout;
                VkDescriptorSet vk_bindless_descriptor_set;

                void buildDescriptorPool(Device &device);
                void buildGlobalDescriptorPool(Device &device);
                void buildBindlessDescriptorPool(Device &device);
                void buildDescriptorSetLayout(Device &device);

            public:
                void build(Device &device);

                void AllocateDescriptorSet(Device& device,DescriptorSetLayout& layout,std::vector<VkDescriptorSet>& sets);
            };

        }
    }
}

#endif