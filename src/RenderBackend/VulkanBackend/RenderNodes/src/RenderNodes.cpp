#include "RenderNodes/RenderNodes.h"
#include "VulkanRender/VulkanRender.h"
#include "CanellaUtility/CanellaUtility.h"
#include <algorithm> // std::min

int sphere_cullig(glm::vec3 center, float radius, glm::vec4 frustum[6])

{
    bool visible = true;
    for (int i = 0; i < 6; ++i)
        visible = visible && glm::dot(frustum[i], glm::vec4(center, 1)) > -radius;
    return int(!visible);
};

Canella::RenderSystem::VulkanBackend::GeomtryPass::GeomtryPass()
{
    // Setup Query Pools for debugging
    timeQuery.resize(6);
    timeQuery[0].name = "DrawMeshTasks (ms)";
    timeQuery[0].description = "Time for rendering the meshes using mesh shaders and taskshaders";
    timeQuery[1].name = "Task Shader invocations ";
    timeQuery[2].name = "Mesh Shader invocations ";
    timeQuery[3].name = "Clipping Primitives ";
    timeQuery[4].name = "Fragment Shader invocations";
}

// todo move this to somewhere else
VkBufferMemoryBarrier bufferBarrier(VkBuffer buffer, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
{
    VkBufferMemoryBarrier result = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};

    result.srcAccessMask = srcAccessMask;
    result.dstAccessMask = dstAccessMask;
    result.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    result.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    result.buffer = buffer;
    result.offset = 0;
    result.size = VK_WHOLE_SIZE;
    return result;
}

void Canella::RenderSystem::VulkanBackend::GeomtryPass::compute_frustum_culling(Canella::Render *render,
                                                                                VkCommandBuffer &command,
                                                                                VkPipeline &compute_pipeline,
                                                                                Drawables &drawables,
                                                                                int image_index)
{
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto &transform_descriptors = vulkan_renderer->transform_descriptors;
    auto &render_camera_data = vulkan_renderer->render_camera_data;
    auto &pipeline_layouts = vulkan_renderer->cachedPipelineLayouts;
    auto &resource_manager = vulkan_renderer->resources_manager;
    auto &device = vulkan_renderer->device;

    glm::mat4 projection_view = glm::transpose(render_camera_data.projection * render_camera_data.view);
    auto compute_pipeline_layout = pipeline_layouts["CommandProcessor"]->getHandle();

    auto normalize_plane = [](glm::vec4 p)
    { return p / length(glm::vec3(p)); };

    glm::vec4 frustum[6];
    frustum[0] = normalize_plane(projection_view[3] + projection_view[0]);
    frustum[1] = normalize_plane(projection_view[3] - projection_view[0]);
    frustum[2] = normalize_plane(projection_view[3] + projection_view[1]);
    frustum[3] = normalize_plane(projection_view[3] - projection_view[1]);
    frustum[4] = normalize_plane(projection_view[3] + projection_view[2]);
    frustum[5] = normalize_plane(projection_view[3] - projection_view[2]);

    int culled_geomtry = 0;
    for (auto i = 0; i < drawables.size(); ++i)
    {
        auto count_buffer = resource_manager.get_buffer_cached(command_count_buffers[i]);
        vkCmdFillBuffer(command, count_buffer->getBufferHandle(), 0, sizeof(uint32_t), 0);

        VkBufferMemoryBarrier fill_barrier = bufferBarrier(count_buffer->getBufferHandle(), VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
        vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, 0, 1, &fill_barrier, 0, 0);

        auto processed_buffer = resource_manager.get_buffer_cached(draw_indirect_buffers[i]);
        VkDescriptorSet descriptors[2] = {frustum_culling_descriptors[i].descriptor_sets[image_index], transform_descriptors[image_index]};
        vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout, 0, 2, descriptors, 0, nullptr);
        vkCmdPushConstants(command, compute_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(frustum), &frustum[0]);
        vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline);
        vkCmdDispatch(command, uint32_t((drawables[i].meshes.size() + 31) / 32), 1, 1);

        VkBufferMemoryBarrier cull = bufferBarrier(processed_buffer->getBufferHandle(), VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
        vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, 0, 0, 0, 1, &cull, 0, 0);
    }
}

void Canella::RenderSystem::VulkanBackend::GeomtryPass::execute(
    Canella::Render *render,
    VkCommandBuffer &command_buffer,
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

    // Compute Frustum culling
    compute_frustum_culling(render, command_buffer, pipelines["CommandProcessor"]->getPipelineHandle(), drawables, current_frame);

    if (debug_statics)
    {
        vkCmdResetQueryPool(command_buffer, queries.timestamp_pool, 0, 2);
        vkCmdResetQueryPool(command_buffer, queries.statistics_pool, 0, 1);
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
        vkCmdBeginQuery(command_buffer, queries.statistics_pool, 0, 0);
    }

    auto draw_indirect = [&]()
    {
        for (auto i = 0; i < drawables.size(); ++i)
        {
            VkDescriptorSet desc[3] = {global_descriptors[index],
                                       transform_descriptors[index],
                                       descriptors[i].descriptor_sets[index]};

            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layouts[pipeline_layout_name]->getHandle(), 0,
                                    3,
                                    desc,
                                    0,
                                    nullptr);

            auto indirect_buffer = resource_manager.get_buffer_cached(draw_indirect_buffers[i]);
            auto command_count = resource_manager.get_buffer_cached(command_count_buffers[i]);
            auto indirect_size = sizeof(IndirectCommandToCull);

            vulkan_renderer->vkCmdDrawMeshTasksIndirectCountEXT(command_buffer,
                                                                indirect_buffer->getBufferHandle(),
                                                                0,
                                                                command_count->getBufferHandle(),
                                                                0,
                                                                drawables[i].meshes.size(),
                                                                indirect_size);
        }
    };
    draw_indirect();

    if (debug_statics)
    {
        vkCmdEndQuery(command_buffer, queries.statistics_pool, 0);
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
        timeQuery[0].time = time;

        vkGetQueryPoolResults(
            device.getLogicalDevice(),
            queries.statistics_pool,
            0,
            1,
            queries.statistics.size() * sizeof(uint64_t),
            queries.statistics.data(),
            sizeof(uint64_t),
            VK_QUERY_RESULT_64_BIT);

        timeQuery[1].time = queries.statistics[0];
        timeQuery[2].time = queries.statistics[1];
        timeQuery[3].time = queries.statistics[2];
        timeQuery[4].time = queries.statistics[3];
    }
}

void Canella::RenderSystem::VulkanBackend::GeomtryPass::load_transient_resources(Canella::Render *render)
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
    resource_static_meshes.resize(drawables.size());

    // Create the buffer used by this node
    create_resource_buffers(render);
    // Create the indirect draw commands
    create_indirect_commands(render);
    // Writes the descriptorset
    write_descriptorsets_geomtry(render);
    // writes descriptorsets for culling pass
    write_descriptorsets_culling(render);

    if (debug_statics && !post_first_load)
        create_render_query(queries, &vulkan_renderer->device);
    post_first_load = true;
}

void Canella::RenderSystem::VulkanBackend::GeomtryPass::write_outputs()
{
}

void Canella::RenderSystem::VulkanBackend::GeomtryPass::setup_reload_resource_event(
    Canella::Render *vulkan_renderer)
{
    // Register the load event. When we perform window resize of Lose swapchain we need to reload this resources
    std::function<void(Canella::Render * render)> reload_resources = [=](Canella::Render *render)
    { load_transient_resources(render); };
    Event_Handler<Canella::Render *> handler(reload_resources);
    vulkan_renderer->OnLostSwapchain += handler;
    vulkan_renderer->OnEnqueueDrawable += handler;
}

// This is called when we lose swapchain. We need to clear and then reload everything
void Canella::RenderSystem::VulkanBackend::GeomtryPass::clear_render_node(Canella::Render *render)
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

void Canella::RenderSystem::VulkanBackend::GeomtryPass::create_resource_buffers(Canella::Render *render)
{
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto &resource_manager = vulkan_renderer->resources_manager;
    auto &drawables = vulkan_renderer->get_drawables();
    auto i = 0;
    for (auto &drawable : drawables)
    {
        resource_bounds_buffers[i] = resource_manager.create_storage_buffer(sizeof(drawable.meshlet_compositions.bounds[0]) *
                                                                                drawable.meshlet_compositions.bounds.size(),
                                                                            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                                                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                                            &vulkan_renderer->transfer_pool,
                                                                            drawable.meshlet_compositions.bounds.data());

        resource_vertices_buffers[i] = resource_manager.create_storage_buffer((sizeof(Vertex) * drawable.positions.size()), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
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

        std::vector<StaticMeshData> mesh_data;
        auto sphere = compute_sphere_bounding_volume(drawables[i].meshes[0], drawables[0].positions);
        for (auto j = 0; j < drawables[i].meshes.size(); ++j)
        {

            StaticMeshData mesh;
            mesh.center = glm::vec3(sphere.x, sphere.w, sphere.z);
            mesh.radius = sphere.w;
            mesh.vertex_offset = drawables[i].meshes[0].vertex_offset;
            mesh.meshlet_triangles_offset = drawables[0].meshes[0].meshlet_triangle_offset;
            mesh.meshlet_offset = drawables[i].meshes[0].meshlet_offset;
            mesh.meshlet_vertices_offset = drawables[0].meshes[0].meshlet_vertex_offset;
            mesh.index_offset = drawables[i].meshes[0].index_offset;
            mesh.mesh_id = i;
            mesh.meshlet_count = drawables[i].meshes[0].meshlet_count;
            mesh_data.push_back(mesh);
        }

        resource_static_meshes[i] = resource_manager.create_storage_buffer(sizeof(StaticMeshData) * mesh_data.size(),
                                                                           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                           &vulkan_renderer->transfer_pool,
                                                                           mesh_data.data());

        i++;
    }
}

void Canella::RenderSystem::VulkanBackend::GeomtryPass::write_descriptorsets_geomtry(Canella::Render *render)
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
            buffer_infos.resize(7);

            auto meshlets_buffer = resource_manager.get_buffer_cached(resource_meshlet_buffers[i]);
            auto vertices_buffer = resource_manager.get_buffer_cached(resource_vertices_buffers[i]);
            auto bounds_buffer = resource_manager.get_buffer_cached(resource_bounds_buffers[i]);
            auto meshlet_tris = resource_manager.get_buffer_cached(resource_meshlet_triangles[i]);
            auto meshlet_vertices = resource_manager.get_buffer_cached(resource_meshlet_vertices[i]);
            auto indirect_buffer = resource_manager.get_buffer_cached(draw_indirect_buffers[i]);
            auto meshes_buffer = resource_manager.get_buffer_cached(resource_static_meshes[i]);

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

            buffer_infos[6].buffer = meshes_buffer->getBufferHandle();
            buffer_infos[6].offset = static_cast<uint32_t>(0);
            buffer_infos[6].range = meshes_buffer->size;

            // write Descriptors for geometry pass
            resource_manager.write_descriptor_sets(descriptors[i].descriptor_sets[j],
                                                   buffer_infos,
                                                   image_infos,
                                                   true);
        }
    }
}

void Canella::RenderSystem::VulkanBackend::GeomtryPass::write_descriptorsets_culling(Canella::Render *render)
{
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto &descriptor_pool = vulkan_renderer->descriptorPool;
    auto &cached_descriptor_set_layouts = vulkan_renderer->cachedDescriptorSetLayouts;
    auto &resource_manager = vulkan_renderer->resources_manager;
    auto number_of_images = vulkan_renderer->swapChain.get_number_of_images();
    auto &drawables = vulkan_renderer->get_drawables();

    frustum_culling_descriptors.resize(drawables.size());

    for (auto i = 0; i < drawables.size(); ++i)
    {
        frustum_culling_descriptors[i].descriptor_sets.resize(number_of_images);
        for (auto j = 0; j < number_of_images; ++j)
        {
            descriptor_pool.allocate_descriptor_set(vulkan_renderer->device,
                                                    cached_descriptor_set_layouts["CommandProcessor"],
                                                    frustum_culling_descriptors[i].descriptor_sets[j]);

            auto draw_indirect = resource_manager.get_buffer_cached(draw_indirect_buffers[i]);
            auto meshes_buffer = resource_manager.get_buffer_cached(resource_static_meshes[i]);
            auto count_buffers = resource_manager.get_buffer_cached(command_count_buffers[i]);

            std::vector<VkDescriptorBufferInfo> buffer_infos;
            std::vector<VkDescriptorImageInfo> image_infos;
            buffer_infos.resize(3);

            buffer_infos[0].buffer = draw_indirect->getBufferHandle();
            buffer_infos[0].offset = static_cast<uint32_t>(0);
            buffer_infos[0].range = draw_indirect->size;

            buffer_infos[1].buffer = meshes_buffer->getBufferHandle();
            buffer_infos[1].offset = static_cast<uint32_t>(0);
            buffer_infos[1].range = meshes_buffer->size;

            buffer_infos[2].buffer = count_buffers->getBufferHandle();
            buffer_infos[2].offset = static_cast<uint32_t>(0);
            buffer_infos[2].range = count_buffers->size;

            // write Descriptors for geometry pass
            resource_manager.write_descriptor_sets(frustum_culling_descriptors[i].descriptor_sets[j],
                                                   buffer_infos,
                                                   image_infos,
                                                   true);
        }
    }
}

void Canella::RenderSystem::VulkanBackend::GeomtryPass::create_indirect_commands(Canella::Render *render)
{
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto &resource_manager = vulkan_renderer->resources_manager;
    auto &drawables = vulkan_renderer->get_drawables();
    draw_indirect_buffers.resize(drawables.size());
    command_count_buffers.resize(drawables.size());

    for (auto i = 0; i < drawables.size(); ++i)
    {

        draw_indirect_buffers[i] = resource_manager.create_buffer(drawables[i].meshes.size() * sizeof(IndirectCommandToCull),
                                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                                      VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        command_count_buffers[i] = resource_manager.create_buffer(4,
                                                                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                                      VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
                                                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }
}

Canella::RenderSystem::VulkanBackend::GeomtryPass::~GeomtryPass()
{
    vkDestroyQueryPool(device->getLogicalDevice(), queries.timestamp_pool, device->getAllocator());
    vkDestroyQueryPool(device->getLogicalDevice(), queries.statistics_pool, device->getAllocator());
}
