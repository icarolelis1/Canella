#include "RenderNodes/GeometryNode.h"
#include "VulkanRender/VulkanRender.h"
#include "CanellaUtility/CanellaUtility.h"
#include <imgui.h>
#include "Render/Framework.h"
#define STORAGE_SIZE  500

void Canella::RenderSystem::VulkanBackend::GeometryPass::load_transient_resources( Canella::Render *render)
{
    hiz_depth.visibility_first_cleared = false;
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    const auto &drawables = render->get_drawables();
    device = &vulkan_renderer->device;

    if (post_first_load)
        clear_render_node(render);

    if (!post_first_load)
        setup_reload_resource_event(render);

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
    write_descriptorsets_geometry( render );
    //Create Hierarchical Depth Buffer
    build_hierarchical_depth(render);
    // writes descriptorsets for culling pass
    write_descriptorsets_culling(render);

    if (debug_statics && !post_first_load)
        create_render_query(queries, &vulkan_renderer->device,10);
    post_first_load = true;
}

void Canella::RenderSystem::VulkanBackend::GeometryPass::execute(
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
    auto main_color = resource_manager.get_image_cached(renderpasses["basic"]->image_accessors[0][index]);
    auto main_depth = resource_manager.get_image_cached(renderpasses["basic"]->image_accessors[1][index]);

    //Reset Query
    reset_queries( command_buffer );
    // Compute Frustum culling
    if (debug_statics)vkCmdWriteTimestamp(command_buffer,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,queries.timestamp_pool,8);
    compute_frustum_culling(render, command_buffer, pipelines["CommandProcessor"]->getPipelineHandle(), drawables, current_frame);
    if (debug_statics)vkCmdWriteTimestamp(command_buffer,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,queries.timestamp_pool, 9);

    //Move attachemnts layout to optimal layouts
    VkImageMemoryBarrier renderBeginBarriers[] =
    {
            image_barrier( main_color->image, 0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                           VK_IMAGE_ASPECT_COLOR_BIT ),
            image_barrier( main_depth->image, 0, 0, VK_IMAGE_LAYOUT_UNDEFINED,
                           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT ),
    };
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 2, renderBeginBarriers);

    const VkViewport viewport = swapchain.get_view_port();
    const VkRect2D rect_2d = swapchain.get_rect2d();

    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &rect_2d);

    //Indirect Draw Lambda
    auto draw_indirect = [&](RenderPass& current_renderpass,bool clear)
    {
        std::vector<VkClearValue> clear_values = {};
        if(clear)
        {
            clear_values.resize(2);
            clear_values[0].color = {{NORMALIZE_COLOR(13), NORMALIZE_COLOR(13), NORMALIZE_COLOR(33), 1.0f}};
            clear_values[1].depthStencil = {1.f,0};
        }

        if (begin_render_pass)
            current_renderpass.beginRenderPass(command_buffer, clear_values, current_frame,VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,pipelines[pipeline_name]->getPipelineHandle());

        for (auto i = 0; i < drawables.size(); ++i)
        {
            VkDescriptorSet desc[3] = {global_descriptors[index],transform_descriptors[index], descriptors[i].descriptor_sets[index]};
            vkCmdBindDescriptorSets(command_buffer,VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline_layouts["MeshShaderPipeline"]->getHandle(),0,3,
                                    desc,0,nullptr);

            auto indirect_buffer = resource_manager.get_buffer_cached(draw_indirect_buffers[i]);
            auto command_count = resource_manager.get_buffer_cached(command_count_buffers[i]);
            auto indirect_size = sizeof(IndirectCommandToCull);

            SpecializedDrawTasks mesh_specialize;
            mesh_specialize.dyamic  = !drawables[i].is_static;
            mesh_specialize.model_matrix = *drawables[i].model_matrix;

            vkCmdPushConstants(command_buffer,pipeline_layouts["MeshShaderPipeline"]->getHandle(),VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_COMPUTE_BIT,0, sizeof(mesh_specialize),&mesh_specialize);

            vulkan_renderer->vkCmdDrawMeshTasksIndirectCountEXT(
                    command_buffer,indirect_buffer->getBufferHandle(),0,command_count->getBufferHandle(),0,drawables[i].meshes.size() * drawables[i].instance_count,indirect_size);
        }

        if (end_render_pass)
            current_renderpass.endRenderPass(command_buffer);
    };

    //Grab the two renderpasses for early and Late pass
    auto& early_pass = renderpasses["basic"];
    auto& late_pass = renderpasses["LatePass"];

    //First Pass
    vkCmdBeginQuery(command_buffer,queries.statistics_pool,0,0);
    if (debug_statics)vkCmdWriteTimestamp(command_buffer,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT ,queries.timestamp_pool,0);
    draw_indirect(*early_pass.get(), true);
    if(do_occlusion_culling == 1.0f)
    {
        if (debug_statics)vkCmdWriteTimestamp(command_buffer,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,queries.timestamp_pool, 1);
        //Update the Pyramid Depth
        if (debug_statics)vkCmdWriteTimestamp(command_buffer,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,queries.timestamp_pool,2);
        update_hiz_chain(render,command_buffer,index);
        if (debug_statics)vkCmdWriteTimestamp(command_buffer,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,queries.timestamp_pool, 3);
        //Occlusion Culling
        if (debug_statics)vkCmdWriteTimestamp(command_buffer,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,queries.timestamp_pool,4);
        execute_occlusion_culling(render,command_buffer,index);
        if (debug_statics)vkCmdWriteTimestamp(command_buffer,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,queries.timestamp_pool, 5);
        //Late pass
        if (debug_statics)vkCmdWriteTimestamp(command_buffer,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,queries.timestamp_pool,6);
        draw_indirect(*late_pass.get(), true);
        if (debug_statics)vkCmdWriteTimestamp(command_buffer,VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,queries.timestamp_pool, 7);
        //End the Query
    }
    vkCmdEndQuery(command_buffer,queries.statistics_pool,0);
    //Change Pyramid and Swapchain Image Layout
    auto pyramid = resource_manager.get_image_cached(hiz_depth.pyramidImage);
    VkImageMemoryBarrier copyBarriers[] =
                 {
                         image_barrier( main_color->image, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                        VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT ),
                         image_barrier( swapchain.get_images()[index], 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                        VK_IMAGE_ASPECT_COLOR_BIT ),
                 };

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 2,copyBarriers);
    auto extent  = swapchain.getExtent();

    if ( false )
    {
        uint32_t debugPyramidLevel = 0;
        uint32_t levelWidth = std::max(1u, hiz_depth.base_width >> debugPyramidLevel);
        uint32_t levelHeight = std::max(1u, hiz_depth.base_height >> debugPyramidLevel);

        VkImageBlit blitRegion = {};
        blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.srcSubresource.mipLevel = debugPyramidLevel;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.srcOffsets[0] = { 0, 0, 0 };
        blitRegion.srcOffsets[1] = { int32_t(levelWidth), int32_t(levelHeight), 1 };
        blitRegion.dstOffsets[0] = { 0, 0, 0 };
        blitRegion.dstOffsets[1] = { int32_t(extent.width), int32_t(extent.height), 1 };

        vkCmdBlitImage(command_buffer, pyramid->image, VK_IMAGE_LAYOUT_GENERAL, swapchain.get_images()[index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blitRegion, VK_FILTER_NEAREST);
    }
    else
    {
        //Copy the final renderpass output to swapchain image
        VkImageCopy copyRegion = {};
        copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.srcSubresource.layerCount = 1;
        copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.dstSubresource.layerCount = 1;
        copyRegion.extent = { extent.width, extent.height, 1 };

        vkCmdCopyImage(command_buffer, main_color->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchain.get_images()[index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
    }

    VkImageMemoryBarrier presentBarrier = image_barrier( swapchain.get_images()[index], VK_ACCESS_TRANSFER_WRITE_BIT, 0,
                                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                         VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT );
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &presentBarrier);
    vkGetQueryPoolResults(device.getLogicalDevice(),queries.timestamp_pool,0,queries.time_stamps.size(),sizeof(uint64_t) * queries.time_stamps.size(),queries.time_stamps.data(),sizeof(uint64_t),VK_QUERY_RESULT_64_BIT);

    uint32_t count = static_cast<uint32_t>(queries.statistics.size());
    //Query the results from pool
    vkGetQueryPoolResults(
            device.getLogicalDevice(),
            queries.statistics_pool,
            0,
            count,
            count * sizeof(uint64_t),
            queries.statistics.data(),
            sizeof(uint64_t),
            VK_QUERY_RESULT_64_BIT
            );

}


void Canella::RenderSystem::VulkanBackend::GeometryPass::compute_frustum_culling( Canella::Render *render,
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
    auto& swapchain = vulkan_renderer->swapChain;

    auto projection_view = render_camera_data.projection * render_camera_data.view;
    glm::mat4 projection_view_transposed = glm::transpose(projection_view);
    auto compute_pipeline_layout = pipeline_layouts["CommandProcessor"]->getHandle();

    auto normalize_plane = [](glm::vec4 p)
    { return p / length(glm::vec3(p)); };

    CullingData& culling_data = hiz_depth.culling_data;
    culling_data.frustumPlanes[0] = normalize_plane(projection_view_transposed[3] + projection_view_transposed[0]);
    culling_data.frustumPlanes[1] = normalize_plane(projection_view_transposed[3] - projection_view_transposed[0]);
    culling_data.frustumPlanes[2] = normalize_plane(projection_view_transposed[3] + projection_view_transposed[1]);
    culling_data.frustumPlanes[3] = normalize_plane(projection_view_transposed[3] - projection_view_transposed[1]);
    culling_data.frustumPlanes[4] = normalize_plane(projection_view_transposed[3] + projection_view_transposed[2]);
    culling_data.frustumPlanes[5] = normalize_plane(projection_view_transposed[3] - projection_view_transposed[2]);

    auto extent = swapchain.getExtent();
    culling_data.width_height_padding = glm::vec4(float(extent.width),do_occlusion_culling,0,0);

    auto total_geometry_count = 0;
    std::for_each(drawables.begin(),drawables.end(),[&total_geometry_count](auto& d)
    {
        total_geometry_count +=d.meshes.size() * d.instance_count;
    });

    auto min_size = std::max(std::min(total_geometry_count,STORAGE_SIZE),1);

    //Clear Visibility  First at first call of compute_frustum_culling
    auto occlusion_buffer = resource_manager.get_buffer_cached(occlusion_visibility_buffer[image_index]);
    if(!hiz_depth.visibility_first_cleared)
    {
        auto& buffer_handle = occlusion_buffer->getBufferHandle();
        vkCmdFillBuffer(command, buffer_handle, 0, 4 * min_size, 0);
        VkBufferMemoryBarrier fillBarrier = VulkanBackend::buffer_barrier( buffer_handle, VK_ACCESS_TRANSFER_WRITE_BIT,
                                                                           VK_ACCESS_SHADER_READ_BIT |
                                                                           VK_ACCESS_SHADER_WRITE_BIT );
        vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0,
                             0, 1, &fillBarrier, 0,
                             0);

        hiz_depth.visibility_first_cleared = true;
    }

    for (auto i = 0; i < drawables.size(); ++i)
    {
        auto count_buffer = resource_manager.get_buffer_cached(command_count_buffers[i]);

        VkBufferMemoryBarrier prefill_barrier = buffer_barrier( count_buffer->getBufferHandle(),
                                                                VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
                                                                VK_ACCESS_TRANSFER_WRITE_BIT );
        vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, 0, 1, &prefill_barrier, 0, 0);

        vkCmdFillBuffer(command, count_buffer->getBufferHandle(), 0, sizeof(uint32_t), 0);

        VkBufferMemoryBarrier fill_barrier = VulkanBackend::buffer_barrier( count_buffer->getBufferHandle(),
                                                                            VK_ACCESS_TRANSFER_WRITE_BIT,
                                                                            VK_ACCESS_SHADER_READ_BIT |
                                                                            VK_ACCESS_SHADER_WRITE_BIT );
        vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, 0, 1, &fill_barrier, 0, 0);

        auto processed_buffer = resource_manager.get_buffer_cached(draw_indirect_buffers[i]);
        VkDescriptorSet descriptors[2] = { geometry_data_descriptors[i].descriptor_sets[image_index], transform_descriptors[image_index]};
        vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout, 0, 2, descriptors, 0, nullptr);
        vkCmdPushConstants(command, compute_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingData), &culling_data);
        vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline);
        vkCmdDispatch(command, uint32_t((drawables[i].meshes.size()*drawables[i].instance_count+ 31) / 32), 1, 1);

        VkBufferMemoryBarrier cull = VulkanBackend::buffer_barrier( processed_buffer->getBufferHandle(),
                                                                    VK_ACCESS_SHADER_WRITE_BIT,
                                                                    VK_ACCESS_INDIRECT_COMMAND_READ_BIT );
        vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, 0, 0, 0, 1, &cull, 0, 0);
    }
}

void Canella::RenderSystem::VulkanBackend::GeometryPass::reset_queries( VkCommandBuffer &command_buffer ) const {
        vkCmdResetQueryPool( command_buffer, queries.timestamp_pool, 0, queries.time_stamps.size());
        vkCmdResetQueryPool( command_buffer, queries.statistics_pool, 0, queries.statistics.size());
}

void Canella::RenderSystem::VulkanBackend::GeometryPass::execute_occlusion_culling(Canella::Render *render,VkCommandBuffer command,int image_index ) {

    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto &transform_descriptors = vulkan_renderer->transform_descriptors;
    auto &resource_manager = vulkan_renderer->resources_manager;
    auto &device = vulkan_renderer->device;
    auto& drawables = vulkan_renderer->get_drawables();
    auto& pipelines = vulkan_renderer->cachedPipelines;
    auto& layouts = vulkan_renderer->cachedPipelineLayouts;

    auto& occlusion_pipeline = pipelines["OcclusionCulling"]->getPipelineHandle();
    auto occlusion_pipeline_layouts = layouts["OcclusionCulling"]->getHandle();
    auto& globaL_descriptors = vulkan_renderer->global_descriptors;

    for (auto i = 0; i < drawables.size(); ++i)
    {
        auto count_buffer = resource_manager.get_buffer_cached(command_count_buffers[i]);
        //wait for last Write to finish (There is 2 render passes for drawing early and late)
        VkBufferMemoryBarrier prefill_barrier = buffer_barrier( count_buffer->getBufferHandle(),
                                                                VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
                                                                VK_ACCESS_TRANSFER_WRITE_BIT );
        vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, 0, 1, &prefill_barrier, 0, 0);
        //Fill the count buffer with 0
        vkCmdFillBuffer(command, count_buffer->getBufferHandle(), 0, sizeof(uint32_t), 0);

        //Wait for FillBuffer to Finish
        VkBufferMemoryBarrier fill_barrier = VulkanBackend::buffer_barrier( count_buffer->getBufferHandle(),
                                                                            VK_ACCESS_TRANSFER_WRITE_BIT,
                                                                            VK_ACCESS_SHADER_READ_BIT |
                                                                            VK_ACCESS_SHADER_WRITE_BIT );
        vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_TRANSFER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0, 0, 0,1,&fill_barrier, 0,0);

        //Dispatch the Occlusion Culling pass2
        VkDescriptorSet descriptors[3] = { geometry_data_descriptors[i].culling_descriptorset[image_index], transform_descriptors[image_index],globaL_descriptors[image_index]};
        vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_COMPUTE, occlusion_pipeline_layouts, 0, 3, descriptors, 0, nullptr);
        vkCmdPushConstants(command, occlusion_pipeline_layouts, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullingData), &hiz_depth.culling_data);
        vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_COMPUTE,occlusion_pipeline );
        vkCmdDispatch(command, uint32_t((drawables[i].meshes.size() * drawables[i].instance_count + 31) / 32), 1, 1);

        //Wait for the occlusion dispatch to finish writing on the draw_indirect_buffer
        auto occlusion_buffer = resource_manager.get_buffer_cached(occlusion_visibility_buffer[image_index]);
        auto processed_buffer = resource_manager.get_buffer_cached(draw_indirect_buffers[i]);
        VkBufferMemoryBarrier cull [] =
        { VulkanBackend::buffer_barrier( processed_buffer->getBufferHandle(), VK_ACCESS_SHADER_WRITE_BIT,
                                         VK_ACCESS_INDIRECT_COMMAND_READ_BIT ),
          VulkanBackend::buffer_barrier( occlusion_buffer->getBufferHandle(), VK_ACCESS_SHADER_WRITE_BIT,
                                         VK_ACCESS_INDIRECT_COMMAND_READ_BIT )
        };
        vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,0, 0, 0,2, &cull[0],0, 0);
    }
}

void Canella::RenderSystem::VulkanBackend::GeometryPass::output_stats()
{
    if (!debug_statics) return;
    auto n_second_convert = device->timestamp_period / 1000000.0f;
    if(ImGui::Button("Occlusion"))
    {
        if(do_occlusion_culling == 1.0f)
            do_occlusion_culling = 0.0f;
        else if(do_occlusion_culling == 0.0f)
            do_occlusion_culling = 1.0f;
    }
    bool  v= do_occlusion_culling == 1.0f? true : false;
    ImGui::Checkbox("Using Occlusion Culling", &v);

    ImGui::Text("Clipping Prim Inv : %.1f  ",double(queries.statistics[0]));
    ImGui::Text("Mesh Shader inv:  %lf ",double(queries.statistics[1]));
    ImGui::Text("Task Shader Inv : %lf  ",double(queries.statistics[2]));
    ImGui::Text("Early pass %.2f ms",double( (queries.time_stamps[1] - queries.time_stamps[0] )* n_second_convert));
    ImGui::Text("Frustum %.2f  ms",double((queries.time_stamps[9] - queries.time_stamps[8]) * n_second_convert));
    ImGui::Text("Pyramid Update Pass %.2f  ms", double((queries.time_stamps[3] - queries.time_stamps[2]) * n_second_convert));
    ImGui::Text("Occlusion Culling %.2f ms",double((queries.time_stamps[5] - queries.time_stamps[4]) * n_second_convert));
    ImGui::Text("Late pass%.2f ms",double((queries.time_stamps[7] - queries.time_stamps[6]) * n_second_convert));

}


void Canella::RenderSystem::VulkanBackend::GeometryPass::write_outputs()
{
}

void Canella::RenderSystem::VulkanBackend::GeometryPass::setup_reload_resource_event(
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
void Canella::RenderSystem::VulkanBackend::GeometryPass::clear_render_node( Canella::Render *render)
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

void Canella::RenderSystem::VulkanBackend::GeometryPass::create_resource_buffers( Canella::Render *render)
{
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto &resource_manager = vulkan_renderer->resources_manager;
    auto &drawables = vulkan_renderer->get_drawables();
    auto i = 0;
    for (auto &drawable : drawables)
    {
        resource_bounds_buffers[i] = Canella::create_storage_buffer(render,sizeof(drawable.meshlet_compositions.bounds[0]) *drawable.meshlet_compositions.bounds.size(),drawable.meshlet_compositions.bounds.data());
        resource_vertices_buffers[i] =  Canella::create_storage_buffer(render,sizeof(Vertex) * drawable.positions.size(),drawable.positions.data());
        resource_meshlet_buffers[i] = Canella::create_storage_buffer(render,sizeof(drawable.meshlet_compositions.meshlets[0]) * drawable.meshlet_compositions.meshlets.size(),drawable.meshlet_compositions.meshlets.data());
        resource_meshlet_vertices[i] =  Canella::create_storage_buffer(render, sizeof(drawable.meshlet_compositions.meshlet_vertices[0]) * drawable.meshlet_compositions.meshlet_vertices.size(),drawable.meshlet_compositions.meshlet_vertices.data());
        resource_meshlet_triangles[i] = Canella::create_storage_buffer(render,sizeof(drawable.meshlet_compositions.meshlet_triangles[0]) * drawable.meshlet_compositions.meshlet_triangles.size(),drawable.meshlet_compositions.meshlet_triangles.data());

        auto total_geometry_count = 0;
        std::for_each(drawables.begin(),drawables.end(),[&total_geometry_count](auto& d)
        {total_geometry_count +=d.meshes.size() * d.instance_count;});
        std::vector<StaticMeshData> mesh_data(total_geometry_count);

       // auto points = generate_points(drawable.instance_count * drawable.meshes.size());
        auto ind  = 0;
        for(auto  rep = 0 ; rep < drawable.instance_count ; ++rep)
            for (auto j = 0; j < drawable.meshes.size(); ++j)
            {
                //auto sphere = compute_sphere_bounding_volume(drawables[i].meshes[j], drawables[i].positions);
                auto sphere = drawable.meshes[j].bounding_volume;
                StaticMeshData mesh;
                mesh.pos = drawable.instance_data[rep].position_offset;
                mesh.center = glm::vec3(sphere.center.x, sphere.center.y, sphere.center.z);
                mesh.radius = sphere.radius;
                mesh.vertex_offset = drawables[i].meshes[j].vertex_offset;
                mesh.meshlet_triangles_offset = drawables[i].meshes[j].meshlet_triangle_offset;
                mesh.meshlet_offset = drawables[i].meshes[j].meshlet_offset;
                mesh.meshlet_vertices_offset = drawables[i].meshes[j].meshlet_vertex_offset;
                mesh.index_offset = drawables[i].meshes[j].index_offset;
                mesh.mesh_id = i;
                mesh.meshlet_count = drawables[i].meshes[j].meshlet_count;
                mesh_data[ind] = (mesh);
                ind++;
            }

        resource_static_meshes[i] = Canella::create_storage_buffer(render,sizeof(StaticMeshData) * mesh_data.size(), mesh_data.data());
        i++;
    }
}

void Canella::RenderSystem::VulkanBackend::GeometryPass::write_descriptorsets_geometry( Canella::Render *render)
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

void Canella::RenderSystem::VulkanBackend::GeometryPass::write_descriptorsets_culling( Canella::Render *render)
{
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto &descriptor_pool = vulkan_renderer->descriptorPool;
    auto &cached_descriptor_set_layouts = vulkan_renderer->cachedDescriptorSetLayouts;
    auto &resource_manager = vulkan_renderer->resources_manager;
    auto number_of_images = vulkan_renderer->swapChain.get_number_of_images();
    auto &drawables = vulkan_renderer->get_drawables();
    geometry_data_descriptors.resize( drawables.size());

    for (auto i = 0; i < drawables.size(); ++i)
    {
        geometry_data_descriptors[i].descriptor_sets.resize( number_of_images);
        geometry_data_descriptors[i].culling_descriptorset.resize( number_of_images);
        for (auto j = 0; j < number_of_images; ++j)
        {
            descriptor_pool.allocate_descriptor_set( vulkan_renderer->device,cached_descriptor_set_layouts["CommandProcessor"],geometry_data_descriptors[i].descriptor_sets[j]);
            descriptor_pool.allocate_descriptor_set( vulkan_renderer->device,cached_descriptor_set_layouts["OcclusionCulling"],geometry_data_descriptors[i].culling_descriptorset[j]);

            auto draw_indirect = resource_manager.get_buffer_cached(draw_indirect_buffers[i]);
            auto meshes_buffer = resource_manager.get_buffer_cached(resource_static_meshes[i]);
            auto count_buffers = resource_manager.get_buffer_cached(command_count_buffers[i]);
            auto occlusion_visibility = resource_manager.get_buffer_cached(occlusion_visibility_buffer[j]);

            std::vector<VkDescriptorBufferInfo> buffer_infos;
            std::vector<VkDescriptorImageInfo> image_infos;
            buffer_infos.resize(4);

            buffer_infos[0].buffer = draw_indirect->getBufferHandle();
            buffer_infos[0].offset = static_cast<uint32_t>(0);
            buffer_infos[0].range  = draw_indirect->size;
            buffer_infos[1].buffer = meshes_buffer->getBufferHandle();
            buffer_infos[1].offset = static_cast<uint32_t>(0);
            buffer_infos[1].range  = meshes_buffer->size;
            buffer_infos[2].buffer = count_buffers->getBufferHandle();
            buffer_infos[2].offset = static_cast<uint32_t>(0);
            buffer_infos[2].range  = count_buffers->size;
            buffer_infos[3].buffer = occlusion_visibility->getBufferHandle();
            buffer_infos[3].offset = static_cast<uint32_t>(0);
            buffer_infos[3].range  = occlusion_visibility->size;

            // write Descriptors for geometry pass
            resource_manager.write_descriptor_sets( geometry_data_descriptors[i].descriptor_sets[j],buffer_infos,image_infos,true);

            image_infos.resize(1);

            auto pyramid = resource_manager.get_image_cached(hiz_depth.pyramidImage);
            VkDescriptorImageInfo image_info{};
            image_info.sampler =    hiz_depth.regular_sampler;
            image_info.imageView = pyramid->view;
            image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            image_infos[0] = image_info;
            resource_manager.write_descriptor_sets( geometry_data_descriptors[i].culling_descriptorset[j],
                                                    buffer_infos,
                                                    image_infos,
                                                    true);

        }
    }
}

void Canella::RenderSystem::VulkanBackend::GeometryPass::create_indirect_commands( Canella::Render *render)
{
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto &resource_manager = vulkan_renderer->resources_manager;
    auto& swapchain = vulkan_renderer->swapChain;
    auto &drawables = vulkan_renderer->get_drawables();
    draw_indirect_buffers.resize(drawables.size());
    command_count_buffers.resize(drawables.size());

    for (auto i = 0; i < drawables.size(); ++i)
    {

        draw_indirect_buffers[i] = Canella::create_device_buffer(render,drawables[i].instance_count * sizeof(IndirectCommandToCull) *drawables[i].meshes.size());
        command_count_buffers[i] = Canella::create_device_buffer(render,4);
    }

    //Create Occlusion Culling Visibility Buffer
    //This buffer will signal when an object is occluded
    auto number_images = swapchain.get_number_of_images();
    occlusion_visibility_buffer.clear();
    occlusion_visibility_buffer.resize(number_images);

    auto total_geometry_count = 0;
    std::for_each(drawables.begin(),drawables.end(),[&total_geometry_count](auto& d)
    {
        total_geometry_count +=d.meshes.size() * d.instance_count;
    });
    auto size  = std::max(total_geometry_count,STORAGE_SIZE);
    //Create the visibility buffer for all -> drawables ->meshes
    for(auto i = 0 ; i < number_images; ++i )
        occlusion_visibility_buffer[i] = resource_manager.create_buffer(4 * size ,
                                                                        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

}

void Canella::RenderSystem::VulkanBackend::GeometryPass::build_hierarchical_depth( Canella::Render* render) {

    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto &swapchain = vulkan_renderer->swapChain;
    auto &resources = vulkan_renderer->resources_manager;
    auto& layouts = vulkan_renderer->cachedPipelineLayouts;

    //Destroy the HI-Z mips before reloading
    if(post_first_load) //Is true after the initial load. Executed again when swapchain resizes
    {
        //Destroy HI-Z mips
        if(hiz_depth.pyramidImage)
            for(auto i = 0 ; i < hiz_depth.mip_count; ++i)
                vkDestroyImageView(device->getLogicalDevice(),hiz_depth.mip_views[i],device->getAllocator());

        vkDestroyDescriptorUpdateTemplate(device->getLogicalDevice(), hiz_depth.updateTemplate, device->getAllocator());
    }
    //Create Sampler
    if(!post_first_load)
    {
        hiz_depth.max_sampler     = create_sampler( device->getLogicalDevice(), VK_SAMPLER_REDUCTION_MODE_MAX_EXT);
        hiz_depth.regular_sampler = create_sampler(device->getLogicalDevice(),VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE);
    }
    auto previous_pow = [](uint32_t v)
    {
        uint32_t r = 1;
        while(r * 2 < v)
        {
            r*= 2;
        }
        return r;
    };
    auto extent = swapchain.getExtent();
    auto base_width = previous_pow(extent.width);
    auto base_height = previous_pow(extent.height);
    hiz_depth.base_width = base_width;
    hiz_depth.base_height = base_height;
    auto get_image_mips_count = [=](uint32_t w,uint32_t h)
    {
        auto count = 0;
        while(h > 1 || w > 1)
        {
            count++;
            h /= 2;
            w /=2;
        }
        return count;
    };

    uint32_t num_mips = get_image_mips_count(base_width,base_height);
    hiz_depth.mip_count = num_mips;
    hiz_depth.pyramidImage = resources.create_image(device,
                                                    base_width,
                                                    base_height,VK_FORMAT_R32_SFLOAT,VK_IMAGE_TILING_OPTIMAL,
                                                    VK_IMAGE_USAGE_STORAGE_BIT|VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,0,num_mips,
                                                    VK_IMAGE_ASPECT_COLOR_BIT,1,VK_SAMPLE_COUNT_1_BIT);

    //Create a view for each mip
    for(auto i = 0; i < num_mips; ++i)
    {
        VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        createInfo.image = resources.get_image_cached(hiz_depth.pyramidImage)->image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = VK_FORMAT_R32_SFLOAT;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = i;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.layerCount = 1;

        VK_CHECK( vkCreateImageView(device->getLogicalDevice(),
                                    &createInfo,device->getAllocator(),
                                    &hiz_depth.mip_views[i]),"Failed to create HIZ-Depth mip view");
    }

    //Create the push descriptor template

    std::array<VkDescriptorUpdateTemplateEntry,2> entries;
    entries[0].descriptorCount = 1;
    entries[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    entries[0].dstBinding = 0;
    entries[0].stride = sizeof(VkDescriptorImageInfo);
    entries[0].offset = 0;
    entries[0].dstArrayElement = 0;

    entries[1].descriptorCount = 1;
    entries[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    entries[1].dstBinding = 1;
    entries[1].stride = sizeof(VkDescriptorImageInfo);
    entries[1].offset = sizeof(VkDescriptorImageInfo);
    entries[1].dstArrayElement = 0;
    VkDescriptorUpdateTemplateCreateInfo create_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO };

    create_info.descriptorUpdateEntryCount = uint32_t(entries.size());
    create_info.pDescriptorUpdateEntries = entries.data();
    create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR;
    create_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    create_info.pipelineLayout = layouts["OcclusionWrite"]->getHandle();

    VK_CHECK(vkCreateDescriptorUpdateTemplate(device->getLogicalDevice(),
                                              &create_info,
                                              0,
                                              &hiz_depth.updateTemplate),"Failed to create VkDescriptorUpdateTemplateCreateInfo");


}

void Canella::RenderSystem::VulkanBackend::GeometryPass::update_hiz_chain( Canella::Render *render,
                                                                           VkCommandBuffer &command,
                                                                           int image_index) {
    auto vulkan_renderer = (VulkanBackend::VulkanRender *)render;
    auto& renderpasses = vulkan_renderer->renderpassManager;
    auto& resource_manager = vulkan_renderer->resources_manager;
    auto& layouts = vulkan_renderer->cachedPipelineLayouts;
    auto& pipelines = vulkan_renderer->cachedPipelines;
    auto& render_data = vulkan_renderer->render_camera_data;
    //This is main depth image. renderpasses stores all the renderpasses and image attachments.
    auto main_depth_target = resource_manager.get_image_cached(renderpasses.renderpasses["basic"]->image_accessors[1][image_index]);
    auto pyramid_image = resource_manager.get_image_cached(hiz_depth.pyramidImage);

    VkImageMemoryBarrier read_barriers[] =
     {
             image_barrier( pyramid_image->image, 0, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT ),
             image_barrier( main_depth_target->image, 0, VK_ACCESS_SHADER_READ_BIT,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            VK_IMAGE_ASPECT_DEPTH_BIT ),
     };

    vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_DEPENDENCY_BY_REGION_BIT,0, 0, 0,0,2, read_barriers);

    //Calculate the groupcount for given dimension in 2D
    auto group_size = [](uint32_t dimension)
    {
        return (dimension + 32 - 1) / 32;
    };

    vkCmdBindPipeline(command,
                      VK_PIPELINE_BIND_POINT_COMPUTE,
                      pipelines["OcclusionWrite"]->getPipelineHandle());

    for(auto i = 0; i < hiz_depth.mip_count; ++i)
    {
        std::array<VkDescriptorImageInfo,2> image_infos;

        image_infos[0].sampler =  VK_NULL_HANDLE;
        image_infos[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        image_infos[0].imageView = hiz_depth.mip_views[i];

        auto source_depth = i == 0 ? main_depth_target->view : hiz_depth.mip_views[i -1];
        image_infos[1].sampler = hiz_depth.max_sampler;
        image_infos[1].imageLayout =  i == 0 ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL;
        image_infos[1].imageView = source_depth;

        vulkan_renderer->vkCmdPushDescriptorSetWithTemplateKHR(command,hiz_depth.updateTemplate,layouts["OcclusionWrite"]->getHandle(),0, image_infos.data());

        uint32_t current_width = std::max(1u, hiz_depth.base_width >> i);
        uint32_t current_height = std::max(1u, hiz_depth.base_width >> i);

        struct  CurrentMipSize {
            glm::vec4 size;
            glm::vec2 aabb_min;
            glm::vec2 aabb_max;

        }push_data;

        push_data.size.x = current_width;
        push_data.size.y = current_height;

        auto g1 = group_size(current_width);
        auto g2 = group_size(current_height);
        vkCmdPushConstants(command,layouts["OcclusionWrite"]->getHandle(),VK_SHADER_STAGE_COMPUTE_BIT,0, sizeof(push_data),&push_data);
        vkCmdDispatch(command,group_size(current_width),group_size(current_height),1);

        //Wait for the image to be written in the computue shader
        VkImageMemoryBarrier wait_barrier = image_barrier( pyramid_image->image, VK_ACCESS_SHADER_WRITE_BIT,
                                                           VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_GENERAL,
                                                           VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT );
        vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_DEPENDENCY_BY_REGION_BIT,0,0, 0,0, 1,&wait_barrier);
    }
    //Reset the image layout
    VkImageMemoryBarrier depthWriteBarrier = image_barrier( main_depth_target->image, VK_ACCESS_SHADER_READ_BIT,
                                                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                                            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                            VK_IMAGE_ASPECT_DEPTH_BIT );
    vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,VK_DEPENDENCY_BY_REGION_BIT,0, 0,0, 0,1, &depthWriteBarrier);

}

Canella::RenderSystem::VulkanBackend::GeometryPass::~GeometryPass()
{
    if(hiz_depth.pyramidImage)
        for(auto i = 0 ; i < hiz_depth.mip_count; ++i)
            vkDestroyImageView(device->getLogicalDevice(),
                               hiz_depth.mip_views[i],
                               device->getAllocator());

    vkDestroyDescriptorUpdateTemplate(device->getLogicalDevice(), hiz_depth.updateTemplate, device->getAllocator());
    vkDestroySampler( device->getLogicalDevice(), hiz_depth.max_sampler, device->getAllocator());
    vkDestroySampler(device->getLogicalDevice(), hiz_depth.regular_sampler,device->getAllocator());
    vkDestroyQueryPool(device->getLogicalDevice(), queries.timestamp_pool, device->getAllocator());
    vkDestroyQueryPool(device->getLogicalDevice(), queries.statistics_pool, device->getAllocator());
}






















