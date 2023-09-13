#include "VulkanImpl/VulkanImpl.h"

 namespace Canella::RenderSystem::VulkanBackend
{
     uint64_t create_storage_buffer(VulkanBackend::VulkanRender * renderer,size_t size, void* pdata)
    {
        auto &resource_manager = renderer->resources_manager;
        return resource_manager.create_storage_buffer(size,
                                               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                               &renderer->transfer_pool,
                                               pdata);
    }
     uint64_t create_device_buffer(VulkanBackend::VulkanRender * renderer,size_t size)
     {
         auto &resource_manager = renderer->resources_manager;
         return resource_manager.create_buffer(size,
                                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                               VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
     }

     uint64_t create_texture( VulkanBackend::VulkanRender *renderer,const std::string &file ) {
         auto &resource_manager = renderer->resources_manager;
         resource_manager.create_texture(file,&renderer->device, VK_FORMAT_R8G8B8A8_UNORM);
         return 0;

     }
 }

