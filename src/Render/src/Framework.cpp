#include "Render/Framework.h"
#include "VulkanImpl/VulkanImpl.h"

namespace Canella
{
    struct RawImageData
    {
#ifdef VULKAN_RENDER
        VkImage image;
#endif
    };

    uint64_t create_storage_buffer(Render* render,size_t size, void* pdata)
    {
#ifdef VULKAN_RENDER
         return RenderSystem::VulkanBackend::create_storage_buffer((RenderSystem::VulkanBackend::VulkanRender*)render,size,pdata);
#endif
    }

    uint64_t create_device_buffer(Render* render, size_t size)
    {
#ifdef VULKAN_RENDER
        return RenderSystem::VulkanBackend::create_device_buffer((RenderSystem::VulkanBackend::VulkanRender*)render,size);
#endif
    }

    uint64_t create_texture( Render* render, const std::string &file ) {
#ifdef VULKAN_RENDER
        return RenderSystem::VulkanBackend::create_texture((RenderSystem::VulkanBackend::VulkanRender*)render,file);
#endif
    }
}