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
            public:
                void destroy();
                void build(Device *device);
                void allocate_descriptor_set(Device& device,std::shared_ptr<DescriptorSetLayout> layout,
                                             VkDescriptorSet& set);
                void free_descriptorsets(Device& device,const VkDescriptorSet*,uint32_t count);
            private:
                const uint32_t k_bindless_texture_binding = 10;
                const uint32_t k_max_bindless_resources = 1024;
                VkDescriptorPool vk_bindless_descriptorpool;
                VkDescriptorPool vk_global_descriptorpool;
                VkDescriptorSetLayout vk_bindless_layout;
                VkDescriptorSet vk_bindless_descriptor_set;

                void build_descriptor_pool(Device *device);
                void build_global_descriptor_pool(Device &device);
                void build_bindless_descriptor_pool(Device &device);
                void build_descriptor_set_layout(Device &device);
                Device* device;
            };

        }
    }
}

#endif