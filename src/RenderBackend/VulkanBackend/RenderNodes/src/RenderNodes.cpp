#include "RenderNodes/RenderNodes.h"
#include "VulkanRender/VulkanRender.h"
#include "CanellaUtility/CanellaUtility.h"


void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::execute(
                                                                        Canella::Render *render,
                                                                        VkCommandBuffer command_buffer,
                                                                        int index)
{

    auto vulkan_renderer =(VulkanBackend::VulkanRender*)render;
    auto& device = vulkan_renderer->device;
    auto& renderpasses = vulkan_renderer->renderpassManager->renderpasses;
    auto& pipelines = vulkan_renderer->cachedPipelines;
    auto& pipeline_layouts = vulkan_renderer->cachedPipelineLayouts;
    auto& frames = vulkan_renderer->frames;
    auto& swapchain = vulkan_renderer->swapChain;
    auto& global_descriptors = vulkan_renderer->global_descriptors;
    auto current_frame = vulkan_renderer->current_frame;
    auto &resource_manager   = vulkan_renderer->resources_manager;

    std::vector<VkClearValue> clear_values = {};
    clear_values.resize(2);
    clear_values[0].color = {{1.0f, 1.0f, 1.f, 1.0f}};

    //frames[index].commandPool.beginCommandBuffer(&device, command_buffer, true);

    const auto render_pass = renderpasses[renderpass_name];
    if(begin_render_pass)
        render_pass->beginRenderPass(command_buffer, clear_values, current_frame);
    const VkViewport viewport = swapchain.get_view_port();
    const VkRect2D rect_2d = swapchain.get_rect2d();
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &rect_2d);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipelines[pipeline_name]->getPipelineHandle());

    for(auto i = 0 ; i < meshlets.size(); ++i ){
        VkDescriptorSet desc[2] = {
                                    global_descriptors[index],
                                    descriptors[i].descriptor_sets[index]
                                   };

        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline_layouts[pipeline_layout_name]->getHandle(),
                                0,
                                2,
                                &desc[0],
                                0,
                                nullptr);


        vkCmdDrawMeshTasksEXT(command_buffer, meshlets[0].meshlets.size(), 1, 1);
    }

    if(end_render_pass)
        renderpasses[renderpass_name]->endRenderPass(command_buffer);
  //  frames[index].commandPool.endCommandBuffer(command_buffer);
}

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::load_transient_resources(
                                                                                        nlohmann::json&,
                                                                                        Canella::Render *render)
{
    auto vulkan_renderer =(VulkanBackend::VulkanRender*)render;
    auto number_of_images = vulkan_renderer->swapChain.getNumberOfImages();
    const auto& drawables = render->get_drawables();
    meshlets.resize(drawables.size());
    resource_vertices_buffers.resize(drawables.size());
    resource_indices_buffers.resize(drawables.size());
    auto &resource_manager   = vulkan_renderer->resources_manager;
    auto i = 0 ;
    for(const auto& drawable : drawables)
    {

        for(const auto& mesh_data : drawable.meshes)
        {
            //create buffer with vertices
            resource_vertices_buffers[i] =  resource_manager.create_host_visible_buffer(sizeof(glm::vec4) *mesh_data.positions.size(),
                                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                        &vulkan_renderer->transfer_pool,
                                                        mesh_data.positions);

            resource_indices_buffers[i] =  resource_manager.create_host_visible_buffer(sizeof(glm::vec4) *mesh_data.positions.size(),
                                                                               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                                               &vulkan_renderer->transfer_pool,
                                                                               mesh_data.indices);



            load_meshlet(meshlets[i], mesh_data);
        }
        i++;
    }

    resource_meshlet_buffers.resize(meshlets.size());
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

        resource_meshlet_buffers[i] = resource_manager.create_buffer(size,
                                                               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        const auto command = vulkan_renderer->transfer_pool.requestCommandBuffer(
                                                            &vulkan_renderer->device,
                                                            VK_COMMAND_BUFFER_LEVEL_PRIMARY );
        copy_buffer_to(command,
                       staging_buffer,
                       *resource_manager.get_buffer_cached(resource_meshlet_buffers[i]).get(),
                           size,
                       vulkan_renderer->device.getTransferQueueHandle());

        i++;
    }

    descriptors.resize(meshlets.size());
    for(auto i = 0 ; i < meshlets.size(); ++i)
    {
        descriptors[i].descriptor_sets.resize(number_of_images);
        for(auto j = 0 ; j < number_of_images; j++)
        {
            std::vector<VkDescriptorBufferInfo> buffer_infos;
            std::vector<VkDescriptorImageInfo> image_infos;
            buffer_infos.resize(3);
            VkBuffer &meshlets_buffer =
                    resource_manager.get_buffer_cached(resource_meshlet_buffers[i])->getBufferHandle();
            VkBuffer &vertices_buffer =
                    resource_manager.get_buffer_cached(resource_vertices_buffers[i])->getBufferHandle();
            VkBuffer &indices_buffer =
                    resource_manager.get_buffer_cached(resource_indices_buffers[i])->getBufferHandle();
            buffer_infos[0].buffer = meshlets_buffer;
            buffer_infos[0].offset = static_cast<uint32_t>(0);
            buffer_infos[0].range  = sizeof(meshopt_Meshlet);

            buffer_infos[1].buffer = vertices_buffer;
            buffer_infos[1].offset = static_cast<uint32_t>(1);
            buffer_infos[1].range  = sizeof(meshopt_Meshlet);

            buffer_infos[2].buffer = indices_buffer;
            buffer_infos[2].offset = static_cast<uint32_t>(2);
            buffer_infos[2].range  = sizeof(meshopt_Meshlet);

            resource_manager.write_descriptor_sets(descriptors[i].descriptor_sets[j], buffer_infos, image_infos);


        }
    }
}

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::write_outputs() {
}
