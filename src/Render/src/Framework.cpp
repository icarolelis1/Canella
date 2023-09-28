#include "Render/Framework.h"
#include "VulkanImpl/VulkanImpl.h"

namespace Canella
{
    struct RawImageData
    {
#ifdef USE_VULKAN
        VkImage image;
#endif
    };

    uint64_t create_storage_buffer(Render* render,size_t size, void* pdata)
    {
#ifdef USE_VULKAN
         return RenderSystem::VulkanBackend::create_storage_buffer((RenderSystem::VulkanBackend::VulkanRender*)render,size,pdata);
#endif
    }

    uint64_t create_device_buffer(Render* render, size_t size)
    {
#ifdef USE_VULKAN
        return RenderSystem::VulkanBackend::create_device_buffer((RenderSystem::VulkanBackend::VulkanRender*)render,size);
#endif
    }

    uint64_t create_texture( Render* render, const std::string &file ) {
#ifdef USE_VULKAN
        return RenderSystem::VulkanBackend::create_texture((RenderSystem::VulkanBackend::VulkanRender*)render,file);
#endif
    }


    void allocate_material_data(Render* render,MaterialData& material)
    {
#ifdef USE_VULKAN
        return RenderSystem::VulkanBackend::allocate_material_data(material,(RenderSystem::VulkanBackend::VulkanRender*)render);
#endif
    }

    uint64_t create_ktx_cube_map(Render* render,const std::string& path)
    {
#ifdef USE_VULKAN
        return RenderSystem::VulkanBackend::create_ktx_cube_map( (RenderSystem::VulkanBackend::VulkanRender*)render,path );
#endif
    }
}