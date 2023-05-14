#include "RenderNodes/RenderNodes.h"
#include "VulkanRender/VulkanRender.h"
#include "CanellaUtility/CanellaUtility.h"

Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::MeshletGBufferPass(
        Canella::RenderSystem::VulkanBackend::NodeType nodeType, const std::string &name,
        const Canella::RenderSystem::VulkanBackend::ResourcesRef &inputs,
        const Canella::RenderSystem::VulkanBackend::ResourcesRef &outputs,
        const Canella::RenderSystem::VulkanBackend::ResourcesRef &transient) : RenderNode(nodeType,
                                                                                          name,
                                                                                          inputs,
                                                                                          outputs,
                                                                                          transient) {}

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::execute(Canella::Render *render)
{
    auto vulkan_renderer =(VulkanBackend::VulkanRender*)render;
    auto& renderpasses = vulkan_renderer->renderpassManager->renderpasses;
    auto& pipelines = vulkan_renderer->cachedPipelines;
}

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::load_transient_resources(
                                                                                        nlohmann::json&,
                                                                                        Canella::Render *render)
{
    auto vulkan_renderer =(VulkanBackend::VulkanRender*)render;
    auto id =uuid();
    const auto& drawables = render->get_drawables();
    meshlets.resize(drawables.size());
    auto i = 0 ;
    for(const auto& drawable : drawables)
    {
        for(const auto& mesh_data : drawable.meshes)
            load_meshlet(meshlets[i], mesh_data);
        i++;
    }
    auto &resource_manager   = vulkan_renderer->resources_manager;
    resource_accessors.resize(meshlets.size());
    i = 0;
    VkDeviceSize  size = sizeof(meshlets[0]) * meshlets.size();
    for(auto& meshlet : meshlets){

        //staging buffer will be destroyed immediatly after transfer
        auto staging_buffer = Buffer(&vulkan_renderer->device,
                                     size,
                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        staging_buffer.udpate(meshlets);

        resource_accessors[i] = resource_manager.create_buffer(size,
                                                               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        const auto command = vulkan_renderer->transfer_pool.requestCommandBuffer(
                                                            &vulkan_renderer->device,
                                                            VK_COMMAND_BUFFER_LEVEL_PRIMARY );
        copy_buffer_to(command,
                       staging_buffer,
                       *resource_manager.get_buffer_cached(resource_accessors[i]).get(),
                       size,
                       vulkan_renderer->device.getTransferQueueHandle());
        i++;
    }
}

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::load_render_node(nlohmann::json &json) {
    RenderNode::load_render_node(json);
}
