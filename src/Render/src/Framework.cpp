#include "Render/Framework.h"
#include "VulkanImpl/VulkanImpl.h"

namespace Canella
{
    uint64_t create_storage_buffer(Render* render,size_t size, void* pdata)
    {
        switch(render->api)
        {
            case Vulkan:
                return RenderSystem::VulkanBackend::create_storage_buffer((RenderSystem::VulkanBackend::VulkanRender*)render,size,pdata);
        }
    }

    uint64_t create_device_buffer(Render* render, size_t size)
    {
        switch(render->api)
        {
            case Vulkan:
                return RenderSystem::VulkanBackend::create_device_buffer((RenderSystem::VulkanBackend::VulkanRender*)render,size);
        }    }
}