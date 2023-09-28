#include "VulkanRender/VulkanRender.h"
#include <core/compressed_pair.hpp>
#include <functional>
#define MAX_FRAMES_IN_FLIGHT 3

using namespace Canella::RenderSystem::VulkanBackend;
void VulkanRender::set_windowing(Windowing *windowing)
{
    window = windowing;
}
/**
 * \brief Creates the VulkanRender
 */
VulkanRender::VulkanRender() : resources_manager(&this->device) {}

void VulkanRender::build( nlohmann::json &config, OnOutputStatsEvent* display_event)
{
    init_vulkan_instance();
    api = GraphicsApi::Vulkan;
    display_render_stats_event = display_event;
    auto glfw_window = dynamic_cast<GlfwWindow *>(window);
    glfw_window->getSurface(instance->handle, &surface);
    const auto [width, height] = dynamic_cast<GlfwWindow *>(window)->getExtent();
    device.prepareDevice(surface, *instance);
    swapChain.prepare_swapchain(width,height,device,surface,VK_FORMAT_B8G8R8A8_UNORM,dynamic_cast<GlfwWindow *>(window)->getHandle(),device.getQueueSharingMode());
    resources_manager.build();
    renderpassManager.build(&device,&swapChain,config["RenderPath"].get<std::string>().c_str(),&resources_manager);
    cache_pipelines(config["Pipelines"].get<std::string>().c_str());
    init_descriptor_pool();
    setup_frames();
    create_global_buffers();
    allocate_global_descriptorsets();
    write_global_descriptorsets();
    setup_internal_renderer_events();
    render_graph.load_render_graph(config["RenderGraph"].get<std::string>().c_str(), this);
    get_device_proc();
    transfer_pool.build(&device, POOL_TYPE::TRANSFER, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    command_pool.build(&device, POOL_TYPE::GRAPHICS, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 10;
    samplerInfo.minLod = 0;
    samplerInfo.maxLod = 11;

    vkCreateSampler(device.getLogicalDevice(),&samplerInfo,device.getAllocator(),&default_sampler);
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.maxLod = 8;
    vkCreateSampler(device.getLogicalDevice(),&samplerInfo,device.getAllocator(),&cube_sampler);

    generate_brdf_lut();
}

void VulkanRender::get_device_proc() {
    vkCmdDrawMeshTasksEXT                 = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr( device.getLogicalDevice(), "vkCmdDrawMeshTasksEXT"));
    vkCmdDrawMeshTasksIndirectEXT         = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectEXT>(vkGetDeviceProcAddr(
            device.getLogicalDevice(), "vkCmdDrawMeshTasksIndirectEXT"));
    vkCmdDrawMeshTasksIndirectCountEXT    = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectCountEXT>(vkGetDeviceProcAddr(
            device.getLogicalDevice(), "vkCmdDrawMeshTasksIndirectCountEXT"));
    vkCmdPushDescriptorSetWithTemplateKHR = reinterpret_cast<PFN_vkCmdPushDescriptorSetWithTemplateKHR>(vkGetDeviceProcAddr(
            device.getLogicalDevice(), "vkCmdPushDescriptorSetWithTemplateKHR"));
}

/**
 * \brief Initialize vulkan instance
 */
void VulkanRender::init_vulkan_instance()
{
    DebugLayers debugger;
    instance = new Instance(debugger, true);
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    debugger.setDebugerMessenger(createInfo, instance->handle);
}

void VulkanRender::enqueue_drawables(Drawables &drawables)
{
    m_drawables = drawables;
}

void VulkanRender::create_render_graph_resources() {
    render_graph.load_resources(this);
    create_transform_buffers();
}


void VulkanRender::create_transform_buffers()
{ // Loads the model matrix
    std::vector<glm::mat4> models;
    if(m_drawables.size() == 0) return;
    for (auto &drawable : m_drawables)
        models.push_back(*drawable.model_matrix);

    // Create the Transform Storage Buffers
    auto number_of_images = swapChain.get_number_of_images();
    transform_descriptors.resize(number_of_images);
    for (auto i = 0; i < number_of_images; ++i)
    {
        transform_buffers.push_back(resources_manager.create_storage_buffer(sizeof(glm::mat4) * MAX_OBJECTS_TRANSFORM,
                                                                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                                            &transfer_pool,
                                                                            models.data()));
    }
    for (auto &descriptor : transform_descriptors)
    {
        descriptorPool.allocate_descriptor_set(
            device,
            cachedDescriptorSetLayouts["Transforms"],
            descriptor);
    }

    auto i = 0;
    for (auto &buffer : transform_buffers)
    {
        std::vector<VkDescriptorBufferInfo> buffer_infos;
        std::vector<VkDescriptorImageInfo> image_infos;
        RefBuffer transform_buffer = resources_manager.get_buffer_cached(buffer);
        buffer_infos.resize(1);
        buffer_infos[0].buffer = transform_buffer->getBufferHandle();
        buffer_infos[0].offset = static_cast<uint32_t>(0);
        buffer_infos[0].range = sizeof(glm::mat4) * m_drawables.size();

        resources_manager.write_descriptor_sets(transform_descriptors[i],
                                                buffer_infos,
                                                image_infos,
                                                true);
        i++;
    }
}

void VulkanRender::render(glm::mat4 &view,glm::vec3& eye, glm::mat4 &projection)
{
    FrameData &frame_data = frames[current_frame];
    vkWaitForFences(device.getLogicalDevice(), 1, &frame_data.imageAvaibleFence, VK_TRUE, UINT64_MAX);
    uint32_t next_image_index;
    VkResult result = vkAcquireNextImageKHR(device.getLogicalDevice(),
                                            swapChain.get_swap_chain_handle(),
                                            UINT64_MAX,
                                            frame_data.imageAcquiredSemaphore,
                                            VK_NULL_HANDLE,
                                            &next_image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        OnLostSwapchain.invoke(this);
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swap chain image!");

    vkResetFences(device.getLogicalDevice(), 1, &frame_data.imageAvaibleFence);
    update_view_projection( view,eye, projection, current_frame );

    record_command_index(frame_data, next_image_index);

    if(enqueue_new_mesh)
    {
        for(auto& mesh : drawables_to_be_inserted)
            m_drawables.push_back(mesh);
        drawables_to_be_inserted.clear();
        should_reload = 1;
        enqueue_new_mesh = false;
    }

    queued_semaphores.push_back(frame_data.imageAcquiredSemaphore);
    wait_stages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = uint32_t(queued_semaphores.size());
    submit_info.pWaitSemaphores    = queued_semaphores.data();
    submit_info.pWaitDstStageMask  = wait_stages.data();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &frame_data.commandBuffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &frame_data.renderFinishedSemaphore;

    const VkSwapchainKHR swap_chains[] = {swapChain.get_swap_chain_handle()};
    vkQueueSubmit(device.getGraphicsQueueHandle(), 1, &submit_info, frame_data.imageAvaibleFence);

    VkPresentInfoKHR present_info      = {};
    present_info.sType                 = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount    = 1;
    present_info.pWaitSemaphores       = &frame_data.renderFinishedSemaphore;
    present_info.swapchainCount        = 1;
    present_info.pSwapchains           = swap_chains;
    present_info.pImageIndices         = &next_image_index;

    result = vkQueuePresentKHR(device.getGraphicsQueueHandle(), &present_info);


    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || should_reload != 0)
    {
        //queued_semaphores.clear();
        OnLostSwapchain.invoke(this);
        should_reload--;
    }

    else if (result != VK_SUCCESS)
        throw std::runtime_error("failed to present swap chain image!");

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
    queued_semaphores.clear();
    enqueue_new_mesh = false;
    wait_stages.clear();

}

void VulkanRender::update_view_projection( glm::mat4 &view,glm::vec3& eye, glm::mat4 &projection, uint32_t next_image_index ) {
    auto refBuffer = resources_manager.get_buffer_cached( global_buffers[next_image_index]);
    render_camera_data.projection = projection;
    render_camera_data.view       = view;
    ViewProjection view_projection{};
    view_projection.view_projection = projection * view;
    view_projection.eye             =-view[3];
    view_projection.view            = view;
    view_projection.projection      = projection;
    auto v = view_projection.eye;
    refBuffer->udpate(view_projection);
}

void VulkanRender::init_descriptor_pool()
{
    descriptorPool.build(&device);
}

void VulkanRender::setup_frames()
{
    const auto number_of_images = swapChain.get_number_of_images();
    for (int i = 0; i < number_of_images; ++i)
    {
        frames.emplace_back();
        frames[i].build(&device);
    }
}

void VulkanRender::cache_pipelines(const char *pipelines)
{
    std::fstream f_stream(pipelines);
    nlohmann::json pipeline_data;
    f_stream >> pipeline_data;
    std::vector<VkPushConstantRange> pushConstants;
    PipelineBuilder::cache_pipeline_data(&device, pipeline_data, cachedDescriptorSetLayouts,
                                         cachedPipelineLayouts, renderpassManager.renderpasses, cachedPipelines);
}

void VulkanRender::create_global_buffers()
{
    const auto number_of_images = swapChain.get_number_of_images();
    for (auto i = 0; i < number_of_images; i++)
    {
        global_buffers.push_back(resources_manager.create_buffer(sizeof(ViewProjection),
                                                                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
    }
}

void VulkanRender::write_global_descriptorsets()
{
    auto i = 0;
    for (auto &buffer : global_buffers)
    {
        std::vector<VkDescriptorBufferInfo> buffer_infos;
        std::vector<VkDescriptorImageInfo> image_infos;
        RefBuffer refBuffer = resources_manager.get_buffer_cached(buffer);
        buffer_infos.resize(1);
        buffer_infos[0].buffer = refBuffer->getBufferHandle();
        buffer_infos[0].offset = static_cast<uint32_t>(0);
        buffer_infos[0].range = sizeof(ViewProjection);

        DescriptorSet::update_descriptorset(&device, global_descriptors[i], buffer_infos, image_infos, false);
        i++;
    }
}

void VulkanRender::record_command_index(FrameData& frame, uint32_t index)
{
    // Execute the render graph
    frame.commandPool.begin_command_buffer(&device, frame.commandBuffer, true);
    render_graph.execute(frame.commandBuffer, this, index);
#if RENDER_EDITOR_LAYOUT
    OnRecordCommandEvent.invoke(frame.commandBuffer, index);
#endif
    frame.commandPool.endCommandBuffer(frame.commandBuffer);
}

void VulkanRender::allocate_global_descriptorsets()
{
    global_descriptors.resize(swapChain.get_number_of_images());
    for (auto &global_descriptor : global_descriptors)
    {
        descriptorPool.allocate_descriptor_set(
            device,
            cachedDescriptorSetLayouts["ViewProjection"],
            global_descriptor);
    }
}

void VulkanRender::destroy()
{
    vkDeviceWaitIdle(device.getLogicalDevice());
    resources_manager.async_loader.destroy();
    resources_manager.async_loader2.destroy();
    resources_manager.resource_loader_pool.destroy(&device);
    transfer_pool.destroy(&device);
    command_pool.destroy(&device);
    free(instance);
    for (auto &frame : frames)
        frame.destroy();
    render_graph.destroy_render_graph();
    swapChain.destroy_swapchain(device);
    renderpassManager.destroy_renderpasses();
    destroy_descriptor_set_layouts();
    destroy_pipeline_layouts();
    destroy_pipelines();
    descriptorPool.destroy();
    resources_manager.destroy_non_persistent_resources();
    resources_manager.destroy_texture_resources();
    vkDestroySampler( device.getLogicalDevice(), default_sampler, device.getAllocator());
    vkDestroySampler( device.getLogicalDevice(), cube_sampler, device.getAllocator());
    device.destroyDevice();
    Canella::Logger::Info("Vulkan Renderer Destroyed!");
}

void VulkanRender::destroy_pipeline_layouts()
{
    auto it = cachedPipelineLayouts.begin();
    while (it != cachedPipelineLayouts.end())
    {
        it->second->destroy(&device);
        it++;
    }
}

void VulkanRender::destroy_descriptor_set_layouts()
{
    auto it = cachedDescriptorSetLayouts.begin();
    while (it != cachedDescriptorSetLayouts.end())
    {
        it->second->destroy(&device);
        it++;
    }
}

void VulkanRender::destroy_pipelines()
{
    auto it = cachedPipelines.begin();
    while (it != cachedPipelines.end())
    {
        it->second->destroy();
        it++;
    }
}

Canella::Drawables &VulkanRender::get_drawables()
{
    return m_drawables;
}

void VulkanRender::setup_internal_renderer_events()
{

    // Handlers for the function events
    std::function<void(Canella::Render *)> reload_fn_pass_manager = [=](Canella::Render *)
    {
        vkDeviceWaitIdle(device.getLogicalDevice());

        // Get Window Surface
        auto glfw_window = dynamic_cast<GlfwWindow *>(window);
        glfw_window->getSurface(instance->handle, &surface);
        const auto [width, height] = dynamic_cast<GlfwWindow *>(window)->getExtent();

        // If window is minimized wait until we focus
        glfw_window->wait_idle();

        // Destroy Objects
        renderpassManager.destroy_renderpasses();
        swapChain.destroy_swapchain(device);
        resources_manager.destroy_non_persistent_resources();
        // resources_manager.async_loader.destroy();
        // resources_manager.build();
        global_buffers.clear();
        transform_buffers.clear();
        // Rebuld Swapchain
        swapChain.prepare_swapchain(width, height, device, surface, VK_FORMAT_B8G8R8A8_UNORM,dynamic_cast<GlfwWindow *>(window)->getHandle(),device.getQueueSharingMode());
        // Free the descriptorsets
        descriptorPool.free_descriptorsets(device, global_descriptors.data(), static_cast<uint32_t>(global_descriptors.size()));
        global_descriptors.clear();
        transform_descriptors.clear();
        // Rebuild Frames command pool and semaphores
        for (auto &frame : frames)frame.rebuild();
        // Rebuild Renderpasses
        renderpassManager.rebuild(&device, &swapChain, &resources_manager);
        // Recreate the uniform buffers
        create_global_buffers();
        // Allocate the descriptorsets for global uniforms
        allocate_global_descriptorsets();
        // Write the descriptorsets
        write_global_descriptorsets();
        // recreate the transform buffers
        create_transform_buffers();
    };
    std::function<void(Extent)> resize_fn = [=](Extent extent)
    {
       this->should_reload = 1;
    };
    Event_Handler<Canella::Render *> reload_renderpass_manager(reload_fn_pass_manager);

    Event_Handler<Extent> reload_on_resize(resize_fn);

    std::function<void(VkSemaphore &,VkPipelineStageFlagBits)> submite_transfer = [=](VkSemaphore &semaphore,VkPipelineStageFlagBits stage)
    {
        enqueue_waits(semaphore,stage);
    };

    Event_Handler<VkSemaphore &,VkPipelineStageFlagBits> submite_transfer_command_handler(submite_transfer);

    resources_manager.OnTransferCommand += submite_transfer_command_handler;
    OnLostSwapchain += reload_renderpass_manager;
    OnEnqueueDrawable += reload_fn_pass_manager;
    auto glfw_window = dynamic_cast<GlfwWindow *>(window);
    glfw_window->OnWindowResize += reload_on_resize;
}

void VulkanRender::enqueue_waits(VkSemaphore semaphore,VkPipelineStageFlagBits stage)
{
    queued_semaphores.push_back(semaphore);
    wait_stages.push_back(stage);
}

VkCommandBuffer VulkanRender::request_command_buffer(VkCommandBufferLevel level)
{
    return command_pool.requestCommandBuffer(&device, level);
}

void VulkanRender::end_command_buffer(VkCommandBuffer cmd)
{
    command_pool.endCommandBuffer(cmd);
}

void VulkanRender::begin_command_buffer(VkCommandBuffer cmd)
{
    command_pool.begin_command_buffer(&device, cmd, true);
}

void Canella::RenderSystem::VulkanBackend::VulkanRender::enqueue_drawable(ModelMesh &mesh)
{
    drawables_to_be_inserted.push_back(mesh);
    enqueue_new_mesh = true;
    Canella::Logger::Debug("%d drawables to insert", drawables_to_be_inserted.size());
}

void VulkanRender::allocate_material( Canella::MaterialData &material ) {

    std::vector<VkDescriptorImageInfo> image_infos;
    std::vector<VkDescriptorBufferInfo> buffer_infos;
    VkDescriptorSet  set;

    for(auto& image : material.texture_accessors)
    {
        descriptorPool.allocate_descriptor_set(device,cachedDescriptorSetLayouts["Material"],set);

        auto image_ref   = resources_manager.get_texture_cached(image);
        VkDescriptorImageInfo descriptor_image_info = {};
        descriptor_image_info.sampler     = default_sampler;
        descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptor_image_info.imageView   = image_ref->view;
        image_infos.push_back( descriptor_image_info);
    }
    DescriptorSet::update_descriptorset(&device,set, buffer_infos, image_infos, false);
    auto pair = std::make_pair(material.name,set);
    raw_materials.push_back(pair);
}

void VulkanRender::generate_brdf_lut() {

    VkDescriptorUpdateTemplate descriptor_update_template;
     brdflut = resources_manager.create_image( &device,
                            512,
                            512, VK_FORMAT_R32G32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_STORAGE_BIT|VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0, 1,
                            VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_SAMPLE_COUNT_1_BIT,true);

    VkDescriptorUpdateTemplateEntry entry;
    entry.descriptorCount = 1;
    entry.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    entry.dstBinding = 0;
    entry.stride = sizeof(VkDescriptorImageInfo);
    entry.offset = 0;
    entry.dstArrayElement = 0;
    VkDescriptorUpdateTemplateCreateInfo create_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO };
    create_info.descriptorUpdateEntryCount = 1;
    create_info.pDescriptorUpdateEntries = &entry;
    create_info.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR;
    create_info.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    create_info.pipelineLayout = cachedPipelines["BrdfLutGen"]->get_pipeline_layout().get_handle();
    auto image = resources_manager.get_texture_cached(brdflut);
    VK_CHECK(vkCreateDescriptorUpdateTemplate(device.getLogicalDevice(),
                                              &create_info,
                                              0,
                                              &descriptor_update_template),
                                              "Failed to create VkDescriptorUpdateTemplateCreateInfo");

    auto command_buffer = request_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    begin_command_buffer(command_buffer);


    auto barrier = image_barrier( image->image,
                                  0,
                                  VK_ACCESS_SHADER_READ_BIT,
                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_GENERAL,
                                  VK_IMAGE_ASPECT_COLOR_BIT );

    vkCmdPipelineBarrier(command_buffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT,
                         0,
                         0, 0,
                         0,1,
                         &barrier);

    vkCmdBindPipeline(command_buffer,
                      VK_PIPELINE_BIND_POINT_COMPUTE,
                      cachedPipelines["BrdfLutGen"]->get_pipeline_handle());

    VkDescriptorImageInfo image_info;

    image_info.sampler =  VK_NULL_HANDLE;
    image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    image_info.imageView = resources_manager.get_texture_cached(brdflut)->view;
    vkCmdPushDescriptorSetWithTemplateKHR(command_buffer,
                                          descriptor_update_template,
                                          cachedPipelines["BrdfLutGen"]->get_pipeline_layout().get_handle(),
                                          0,
                                          &image_info);
    //Calculate the groupcount for given dimension in 2D
    auto group_size = [](uint32_t dimension)
    {return (dimension + 32 - 1) / 32;};

    vkCmdDispatch(command_buffer,group_size(512),group_size(512),1);

    auto to_shader_read = image_barrier( image->image,
                                         VK_ACCESS_SHADER_WRITE_BIT,
                                  VK_ACCESS_SHADER_READ_BIT,
                                  VK_IMAGE_LAYOUT_GENERAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                  VK_IMAGE_ASPECT_COLOR_BIT );

    vkCmdPipelineBarrier(command_buffer,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         VK_DEPENDENCY_BY_REGION_BIT,
                         0,0, 0,0,
                         1, &to_shader_read);


    end_command_buffer(command_buffer);
    vkDestroyDescriptorUpdateTemplate(device.getLogicalDevice(), descriptor_update_template, device.getAllocator());

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    vkQueueSubmit(device.getGraphicsQueueHandle(), 1, &submit_info,VK_NULL_HANDLE);
    vkDeviceWaitIdle(device.getLogicalDevice());

}

void VulkanRender::set_environment_maps( Canella::EnvironmentMaps &maps ) {

    descriptorPool.allocate_descriptor_set(device,cachedDescriptorSetLayouts["Environment"],enviroment_set);

    std::vector<VkDescriptorBufferInfo> buffer_infos;
    std::vector<VkDescriptorImageInfo> image_infos;

    auto irradiance = resources_manager.get_texture_cached(maps.irradiance);
    auto specular = resources_manager.get_texture_cached(maps.specular);
    auto brdfLut = resources_manager.get_texture_cached(brdflut);

    VkDescriptorImageInfo irradiance_image_info = {};
    irradiance_image_info.sampler     = cube_sampler;
    irradiance_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    irradiance_image_info.imageView   = irradiance->view;

    VkDescriptorImageInfo specular_image_info = {};
    specular_image_info.sampler     = cube_sampler;
    specular_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    specular_image_info.imageView   = specular->view;

    VkDescriptorImageInfo brdflut_image_info = {};
    brdflut_image_info.sampler     = default_sampler;
    brdflut_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    brdflut_image_info.imageView   = brdfLut->view;

    image_infos.push_back( irradiance_image_info );
    image_infos.push_back( specular_image_info );
    image_infos.push_back( brdflut_image_info );

    resources_manager.write_descriptor_sets(enviroment_set,buffer_infos,image_infos,false);


}

