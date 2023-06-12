#include "RenderNodes/RenderNodes.h"
#include "VulkanRender/VulkanRender.h"
#include "CanellaUtility/CanellaUtility.h"
#include <algorithm> // std::min

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::execute(
    Canella::Render *render,
    VkCommandBuffer command_buffer,
    int index)
{
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto &device = vulkan_renderer->device;
    auto &renderpasses = vulkan_renderer->renderpassManager.renderpasses;
    auto &pipelines = vulkan_renderer->cachedPipelines;
    auto &pipeline_layouts = vulkan_renderer->cachedPipelineLayouts;
    auto &swapchain = vulkan_renderer->swapChain;
    auto &global_descriptors = vulkan_renderer->global_descriptors;
    auto &transform_descriptors = vulkan_renderer->transform_descriptors;
    auto current_frame = index;
    auto &drawables = vulkan_renderer->get_drawables();
    auto &resource_manager = vulkan_renderer->resources_manager;

    std::vector<VkClearValue> clear_values = {};
    clear_values.resize(2);
    clear_values[0].color = {{NORMALIZE_COLOR(70), NORMALIZE_COLOR(70), NORMALIZE_COLOR(70), 1.0f}};
    clear_values[1].depthStencil = {1.0f};

    const auto render_pass = renderpasses[renderpass_name].get();
    if (debug_statics)
    {
        vkCmdResetQueryPool(command_buffer,
                            queries.timestamp_pool,
                            0,
                            2);
    }
    if (begin_render_pass)
        render_pass->beginRenderPass(command_buffer, clear_values, current_frame);

    const VkViewport viewport = swapchain.get_view_port();
    const VkRect2D rect_2d = swapchain.get_rect2d();
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &rect_2d);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipelines[pipeline_name]->getPipelineHandle());
    if (debug_statics)
    {
        vkCmdWriteTimestamp(command_buffer,
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                            queries.timestamp_pool,
                            0);
    }

    auto draw_indirect = [&]()
    {
        for (auto i = 0; i < drawables.size(); ++i)
        {
            VkDescriptorSet desc[3] = {global_descriptors[index],
                                       transform_descriptors[index],
                                       descriptors[i].descriptor_sets[index]};

            vkCmdBindDescriptorSets(command_buffer,
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline_layouts[pipeline_layout_name]->getHandle(),
                                    0,
                                    3,
                                    desc,
                                    0,
                                    nullptr);

            auto indirect_buffer = resource_manager.get_buffer_cached(draw_indirect_buffers[i]);
            auto indirect_size = sizeof(IndirectCommand);
            vulkan_renderer->vkCmdDrawMeshTasksIndirectEXT(command_buffer,
                                                           indirect_buffer->getBufferHandle(),
                                                           0,
                                                           drawables[i].meshes.size(),
                                                           indirect_size);
        }
    };
    draw_indirect();
    // vulkan_renderer->vkCmdDrawMeshTasksEXT(command_buffer,std::ceil(meshlets[i].meshlets.size()/32)+1 , 1, 1);

    if (debug_statics)
    {
        vkCmdWriteTimestamp(command_buffer,
                            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                            queries.timestamp_pool, 1);
    }
    if (end_render_pass)
        renderpasses[renderpass_name]->endRenderPass(command_buffer);
    if (debug_statics)
    {
        vkGetQueryPoolResults(
            device.getLogicalDevice(),
            queries.timestamp_pool,
            0,
            2,
            sizeof(uint64_t) * 2,
            queries.time_stamps.data(),
            sizeof(uint64_t),
            VK_QUERY_RESULT_64_BIT);

        auto time = (queries.time_stamps[1] - queries.time_stamps[0]) *
                    device.timestamp_period / 1000000.0f;
        timeQuery.time = time;
    }
}

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::load_transient_resources(Canella::Render *render)
{
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    const auto &drawables = render->get_drawables();
    device = &vulkan_renderer->device;

    if (post_first_load)
        clear_render_node(render);

    if (!post_first_load)
    {
        setup_reload_resource_event(render);
    }
    resource_vertices_buffers.resize(drawables.size());
    resource_bounds_buffers.resize(drawables.size());
    resource_meshlet_buffers.resize(drawables.size());
    resource_meshlet_triangles.resize(drawables.size());
    resource_meshlet_vertices.resize(drawables.size());
    // Create the buffer used by this node
    create_resource_buffers(render);
    // Create the indirect draw commands
    create_indirect_commands(render);
    // Writes the descriptorset
    write_descriptorsets(render);

    if (debug_statics && !post_first_load)
        create_render_query(queries, &vulkan_renderer->device);
    post_first_load = true;
}

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::write_outputs()
{
}

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::setup_reload_resource_event(
    Canella::Render *vulkan_renderer)
{
    // Register the load event. When we perform window resize of Lose swapchain we need to reload this resources
    std::function<void(Canella::Render * render)> reload_resources = [=](Canella::Render *render)
    { load_transient_resources(render); };
    Event_Handler<Canella::Render *> handler(reload_resources);
    vulkan_renderer->OnLostSwapchain += handler;
}

// This is called when we lose swapchain. We need to clear and then reload everything
void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::clear_render_node(Canella::Render *render)
{
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;

    resource_vertices_buffers.clear();
    resource_bounds_buffers.clear();
    resource_meshlet_buffers.clear();
    resource_meshlet_triangles.clear();
    resource_meshlet_vertices.clear();
    descriptors.clear();

    for (auto &descriptor : descriptors)
        for (auto &descriptorset : descriptor.descriptor_sets)
            vulkan_renderer->descriptorPool.free_descriptorsets(vulkan_renderer->device, &descriptorset, 1);
    // descriptors.clear();
}

Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::MeshletGBufferPass()
{
    timeQuery.name = "DrawMeshTasks (GPU time)";
    timeQuery.description = "Time for rendering the meshes using mesh shaders and taskshaders";
}

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::create_resource_buffers(Canella::Render *render)
{
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto &resource_manager = vulkan_renderer->resources_manager;
    auto &drawables = vulkan_renderer->get_drawables();
    auto i = 0;
    for (auto &drawable : drawables)
    {
        resource_bounds_buffers[i] = resource_manager.create_storage_buffer(
            sizeof(drawable.meshlet_compositions.bounds[0]) *
                drawable.meshlet_compositions.bounds.size(),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            &vulkan_renderer->transfer_pool,
            drawable.meshlet_compositions.bounds.data());

        resource_vertices_buffers[i] = resource_manager.create_storage_buffer(
            (sizeof(Vertex) * drawable.positions.size()),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            &vulkan_renderer->transfer_pool,
            drawable.positions.data());

        resource_meshlet_buffers[i] = resource_manager.create_storage_buffer(sizeof(drawable.meshlet_compositions.meshlets[0]) * drawable.meshlet_compositions.meshlets.size(),
                                                                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                             &vulkan_renderer->transfer_pool,
                                                                             drawable.meshlet_compositions.meshlets.data());

        resource_meshlet_vertices[i] = resource_manager.create_storage_buffer(sizeof(drawable.meshlet_compositions.meshlet_vertices[0]) * drawable.meshlet_compositions.meshlet_vertices.size(),
                                                                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                              &vulkan_renderer->transfer_pool,
                                                                              drawable.meshlet_compositions.meshlet_vertices.data());

        resource_meshlet_triangles[i] = resource_manager.create_storage_buffer(sizeof(drawable.meshlet_compositions.meshlet_triangles[0]) * drawable.meshlet_compositions.meshlet_triangles.size(),
                                                                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                               &vulkan_renderer->transfer_pool,
                                                                               drawable.meshlet_compositions.meshlet_triangles.data());

        i++;
    }
}

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::write_descriptorsets(Canella::Render *render)
{
    // descriptors.resize(meshlet_composition.size());
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto &descriptor_pool = vulkan_renderer->descriptorPool;
    auto &cached_descriptor_set_layouts = vulkan_renderer->cachedDescriptorSetLayouts;
    auto &resource_manager = vulkan_renderer->resources_manager;
    auto number_of_images = vulkan_renderer->swapChain.get_number_of_images();
    auto &drawables = vulkan_renderer->get_drawables();
    descriptors.resize(drawables.size());

    for (auto i = 0; i < drawables.size(); ++i)
    {
        descriptors[i].descriptor_sets.resize(number_of_images);
        for (auto j = 0; j < number_of_images; j++)
        {
            descriptor_pool.allocate_descriptor_set(vulkan_renderer->device,
                                                    cached_descriptor_set_layouts["Meshlets"],
                                                    descriptors[i].descriptor_sets[j]);

            std::vector<VkDescriptorBufferInfo> buffer_infos;
            std::vector<VkDescriptorImageInfo> image_infos;
            buffer_infos.resize(6);

            auto meshlets_buffer = resource_manager.get_buffer_cached(resource_meshlet_buffers[i]);
            auto vertices_buffer = resource_manager.get_buffer_cached(resource_vertices_buffers[i]);
            auto bounds_buffer = resource_manager.get_buffer_cached(resource_bounds_buffers[i]);
            auto meshlet_tris = resource_manager.get_buffer_cached(resource_meshlet_triangles[i]);
            auto meshlet_vertices = resource_manager.get_buffer_cached(resource_meshlet_vertices[i]);
            auto indirect_buffer = resource_manager.get_buffer_cached(draw_indirect_buffers[i]);

            buffer_infos[0].buffer = meshlets_buffer->getBufferHandle();
            buffer_infos[0].offset = static_cast<uint32_t>(0);
            buffer_infos[0].range = meshlets_buffer->size;

            buffer_infos[1].buffer = vertices_buffer->getBufferHandle();
            buffer_infos[1].offset = static_cast<uint32_t>(0);
            buffer_infos[1].range = vertices_buffer->size;

            buffer_infos[2].buffer = bounds_buffer->getBufferHandle();
            buffer_infos[2].offset = static_cast<uint32_t>(0);
            buffer_infos[2].range = bounds_buffer->size;

            buffer_infos[3].buffer = meshlet_tris->getBufferHandle();
            buffer_infos[3].offset = static_cast<uint32_t>(0);
            buffer_infos[3].range = meshlet_tris->size;

            buffer_infos[4].buffer = meshlet_vertices->getBufferHandle();
            buffer_infos[4].offset = static_cast<uint32_t>(0);
            buffer_infos[4].range = meshlet_vertices->size;

            buffer_infos[5].buffer = indirect_buffer->getBufferHandle();
            buffer_infos[5].offset = static_cast<uint32_t>(0);
            buffer_infos[5].range = indirect_buffer->size;

            resource_manager.write_descriptor_sets(descriptors[i].descriptor_sets[j],
                                                   buffer_infos,
                                                   image_infos,
                                                   true);
        }
    }
}

void Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::create_indirect_commands(Canella::Render *render)
{
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto &resource_manager = vulkan_renderer->resources_manager;
    auto &drawables = vulkan_renderer->get_drawables();
    draw_indirect_buffers.resize(drawables.size());

    for (auto i = 0; i < drawables.size(); ++i)
    {
        std::vector<IndirectCommand> commands;
        for (auto j = 0; j < drawables[i].meshes.size(); ++j)
        {
            IndirectCommand ext;
            ext.groupCountX = static_cast<uint32_t>((drawables[i].meshes[j].meshlet_count + 31)/32) ;
            ext.groupCountY = 1;
            ext.groupCountZ = 1;
            ext.vertex_offset = drawables[i].meshes[j].vertex_offset;
            ext.meshlet_triangle_offset = drawables[i].meshes[j].meshlet_triangle_offset;
            ext.meshlet_offset = drawables[i].meshes[j].meshlet_offset;
            ext.meshlet_vertex_offset = drawables[i].meshes[j].meshlet_vertex_offset;
            ext.index_offset = drawables[i].meshes[j].index_offset;
            ext.meshlet_offset = drawables[i].meshes[j].meshlet_offset;
            ext.mesh_id = i;
            ext.meshlet_count = static_cast<uint32_t>(drawables[i].meshes[j].meshlet_count);
            commands.push_back(ext);
        }
        draw_indirect_buffers[i] = resource_manager.create_storage_buffer(
            commands.size() * (sizeof(IndirectCommand)),
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
            &vulkan_renderer->transfer_pool,
            commands.data());
    }
}

Canella::RenderSystem::VulkanBackend::MeshletGBufferPass::~MeshletGBufferPass()
{
    vkDestroyQueryPool(device->getLogicalDevice(), queries.timestamp_pool, device->getAllocator());
}
