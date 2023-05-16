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

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::execute(
                                                                        Canella::Render *render,
                                                                        VkCommandBuffer command_buffer,
                                                                        int index)
{

    auto vulkan_renderer =(VulkanBackend::VulkanRender*)render;
    auto& device = vulkan_renderer->device;
    auto& renderpasses = vulkan_renderer->renderpassManager->renderpasses;
    auto& pipelines = vulkan_renderer->cachedPipelines;
    auto& frames = vulkan_renderer->frames;
    auto& swapchain = vulkan_renderer->swapChain;
    auto& global_descriptors = vulkan_renderer->global_descriptors;
    auto current_frame = vulkan_renderer->current_frame;
    std::vector<VkClearValue> clear_values = {};
    clear_values.resize(2);
    clear_values[0].color = {{1.0f, 1.0f, 1.f, 1.0f}};

    frames[index].commandPool.beginCommandBuffer(&device, command_buffer, true);
    const auto render_pass = renderpasses["basic"];
    render_pass->beginRenderPass(command_buffer, clear_values, current_frame);
    const VkViewport viewport = swapchain.get_view_port();
    const VkRect2D rect_2d = swapchain.get_rect2d();
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &rect_2d);

    VkDescriptorSet descritpros[2] = { global_descriptors[index],meshlet_gpu_resources[0].descriptorsets[index] };
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            cachedPipelineLayouts["MeshShaderPipeline"]->getHandle(),
                            0,
                            2,
                            &descritpros[0],
                            0,
                            nullptr);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      cachedPipelines["MeshShaderPipeline"]->getPipelineHandle());
    vkCmdDrawMeshTasksEXT(command_buffer, 1, 1, 1);
    renderpasses["basic"]->endRenderPass(command_buffer);

    frames[index].commandPool.endCommandBuffer(command_buffer);
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

        auto j = 0;
        for (auto& descriptorset : meshlet_resource.descriptorsets)
        {
            std::vector<VkDescriptorBufferInfo> buffer_infos;
            std::vector<VkDescriptorImageInfo> image_infos;
            buffer_infos.resize(1);
            buffer_infos[0].buffer = meshlet_resource.buffer.getBufferHandle();
            buffer_infos[0].offset = static_cast<uint32_t>(0);
            buffer_infos[0].range = sizeof(meshopt_Meshlet);
            DescriptorSet::update_descriptorset(&device, meshlet_resource.descriptorsets[i], buffer_infos,
                                                image_infos);
            j++;
        }

        i++;
    }
}

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::load_render_node(nlohmann::json &json) {
    RenderNode::load_render_node(json);
}
