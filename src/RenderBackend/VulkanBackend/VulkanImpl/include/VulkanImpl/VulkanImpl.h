#ifndef VULKAN_IMPL
#define VULKAN_IMPL
#include "Render/Framework.h"
#include "VulkanRender/VulkanRender.h"

namespace Canella::RenderSystem::VulkanBackend{
    uint64_t create_storage_buffer(VulkanBackend::VulkanRender * render,size_t size, void* pdata);
    uint64_t create_device_buffer(VulkanBackend::VulkanRender * renderer,size_t size);
    uint64_t create_texture(VulkanBackend::VulkanRender * renderer,const std::string& file );
}

#endif