#include "VulkanRender/VulkanRender.h"
#include <core/compressed_pair.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <functional>

using namespace Canella::RenderSystem::VulkanBackend;

void VulkanRender::set_windowing(Windowing *windowing)
{
    window = windowing;
}
/**
 * \brief Creates the VulkanRender
 */
VulkanRender::VulkanRender() : resources_manager(&this->device) {}

void VulkanRender::build(nlohmann::json &config)
{
    init_vulkan_instance();
    auto glfw_window = dynamic_cast<GlfwWindow *>(window);
    glfw_window->getSurface(instance->handle, &surface);
    const auto [width, height] = dynamic_cast<GlfwWindow *>(window)->getExtent();
    device.prepareDevice(surface, *instance);
    swapChain.prepare_swapchain(width,
                                height,
                                device,
                                surface,
                                VK_FORMAT_B8G8R8A8_UNORM,
                                dynamic_cast<GlfwWindow *>(window)->getHandle(),
                                device.getQueueSharingMode());

    resources_manager.build();

    renderpassManager.build(&device,
                            &swapChain,
                            config["RenderPath"].get<std::string>().c_str(),
                            &resources_manager);

    cache_pipelines(config["Pipelines"].get<std::string>().c_str());
    init_descriptor_pool();
    setup_frames();
    allocate_global_usage_buffers();
    allocate_global_descriptorsets();
    write_global_descriptorsets();
    setup_renderer_events();
    render_graph.load_render_graph(config["RenderGraph"].get<std::string>().c_str(), this);
    vkCmdDrawMeshTasksEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(
        device.getLogicalDevice(),
        "vkCmdDrawMeshTasksEXT"));

    vkCmdDrawMeshTasksIndirectEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectEXT>(vkGetDeviceProcAddr(device.getLogicalDevice(),
                                                                                                            "vkCmdDrawMeshTasksIndirectEXT"));

    vkCmdDrawMeshTasksIndirectCountEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksIndirectCountEXT>(vkGetDeviceProcAddr(device.getLogicalDevice(),
                                                                                                                      "vkCmdDrawMeshTasksIndirectCountEXT"));
    vkCmdPushDescriptorSetWithTemplateKHR = reinterpret_cast<PFN_vkCmdPushDescriptorSetWithTemplateKHR>(vkGetDeviceProcAddr(device.getLogicalDevice(),
                                                                                                                         "vkCmdPushDescriptorSetWithTemplateKHR"));
    transfer_pool.build(&device, POOL_TYPE::TRANSFER, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    command_pool.build(&device, POOL_TYPE::GRAPHICS, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

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
    render_graph.load_resources(this);
    create_transform_buffers();
}

void VulkanRender::create_transform_buffers()
{ // Loads the model matrix
    std::vector<glm::mat4> models;
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

void VulkanRender::render(glm::mat4 &view, glm::mat4 &projection)
{
    FrameData &frame_data = frames[current_frame];
    vkWaitForFences(device.getLogicalDevice(), 1, &frame_data.imageAvaibleFence, VK_TRUE, UINT64_MAX);

    if (dispatch_on_enqueue)
        OnEnqueueDrawable.invoke(this);
    dispatch_on_enqueue = false;

    uint32_t next_image_index;
    VkResult result = vkAcquireNextImageKHR(device.getLogicalDevice(), swapChain.get_swap_chain_handle(), UINT64_MAX, frame_data.imageAcquiredSemaphore,
                                            VK_NULL_HANDLE,
                                            &next_image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        Canella::Logger::Info("--------- CALLING EVENT OnLostSwapchain ---------");
        OnLostSwapchain.invoke(this);
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swap chain image!");

    vkResetFences(device.getLogicalDevice(), 1, &frame_data.imageAvaibleFence);
    auto refBuffer = resources_manager.get_buffer_cached(global_buffers[current_frame]);

    dispatch_on_enqueue             = false;
    render_camera_data.projection   = projection;
    render_camera_data.view         = view;

    ViewProjection view_projection{};
    view_projection.view_projection = projection * view;
    view_projection.eye             = glm::vec4(view[3]);

    refBuffer->udpate(view_projection);
    record_command_index(frame_data.commandBuffer, current_frame);

#if RENDER_EDITOR_LAYOUT
    OnRecordCommandEvent.invoke(frame_data.editor_command, current_frame, frame_data);
#endif
    queued_semaphores.push_back(frame_data.imageAcquiredSemaphore);
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    std::vector<VkPipelineStageFlags> wait_stages;
    wait_stages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    for (auto i = 1; i < queued_semaphores.size(); ++i)
        wait_stages.push_back(VK_PIPELINE_STAGE_TRANSFER_BIT);

    submit_info.waitSemaphoreCount = uint32_t(queued_semaphores.size());
    submit_info.pWaitSemaphores    = queued_semaphores.data();
    submit_info.pWaitDstStageMask  = wait_stages.data();

#if RENDER_EDITOR_LAYOUT
    submit_info.commandBufferCount = 2;
    VkCommandBuffer commands[2] = {frame_data.commandBuffer, frame_data.editor_command};
    submit_info.pCommandBuffers = &commands[0];
#else
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &frame_data.commandBuffer;
#endif
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &frame_data.renderFinishedSemaphore;

    (vkQueueSubmit(device.getGraphicsQueueHandle(), 1, &submit_info, frame_data.imageAvaibleFence));

    VkPresentInfoKHR present_info      = {};
    present_info.sType                 = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount    = 1;
    present_info.pWaitSemaphores       = &frame_data.renderFinishedSemaphore;
    const VkSwapchainKHR swap_chains[] = {swapChain.get_swap_chain_handle()};
    present_info.swapchainCount        = 1;
    present_info.pSwapchains           = swap_chains;
    present_info.pImageIndices         = &next_image_index;


    result = vkQueuePresentKHR(device.getGraphicsQueueHandle(), &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        Canella::Logger::Info("--------- CALLING EVENT OnLostSwapchain ---------");
        OnLostSwapchain.invoke(this);
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
    current_frame = (current_frame + 1) % 3;
    queued_semaphores.clear();
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

void VulkanRender::allocate_global_usage_buffers()
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

void VulkanRender::record_command_index(VkCommandBuffer &commandBuffer, uint32_t index)
{
    // Execute the render graph
    frames[index].commandPool.begin_command_buffer(&device, commandBuffer, true);
    render_graph.execute(commandBuffer, this, index);
    frames[index].commandPool.endCommandBuffer(commandBuffer);
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
    resources_manager.destroy_resources();
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

void VulkanRender::setup_renderer_events()
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
        resources_manager.destroy_resources();
        // resources_manager.async_loader.destroy();
        // resources_manager.build();
        global_buffers.clear();
        transform_buffers.clear();
        // Rebuld Swapchain
        swapChain.prepare_swapchain(width, height, device, surface, VK_FORMAT_B8G8R8A8_UNORM,
                                    dynamic_cast<GlfwWindow *>(window)->getHandle(),
                                    device.getQueueSharingMode());

        // Free the descriptorsets
        descriptorPool.free_descriptorsets(device, global_descriptors.data(), static_cast<uint32_t>(global_descriptors.size()));
        global_descriptors.clear();
        transform_descriptors.clear();
        // Rebuild Frames command pool and semaphores
        for (auto &frame : frames)
            frame.rebuild();
        // Rebuild Renderpasses
        renderpassManager.rebuild(&device, &swapChain, &resources_manager);
        // Recreate the uniform buffers
        allocate_global_usage_buffers();
        // Allocate the descriptorsets for global uniforms
        allocate_global_descriptorsets();
        // Write the descriptorsets
        write_global_descriptorsets();
        // recreate the transform buffers
        create_transform_buffers();
    };
    Event_Handler<Canella::Render *> reload_renderpass_manager(reload_fn_pass_manager);

    std::function<void(VkSemaphore &)> submite_transfer = [=](VkSemaphore &semaphore)
    {
        enqueue_waits(semaphore);
    };
    Event_Handler<VkSemaphore &> submite_transfer_command_handler(submite_transfer);

    resources_manager.OnTransferCommand += submite_transfer_command_handler;
    OnLostSwapchain += reload_renderpass_manager;
    OnEnqueueDrawable += reload_fn_pass_manager;
}

void VulkanRender::enqueue_waits(VkSemaphore semaphore)
{
    queued_semaphores.push_back(semaphore);
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
    m_drawables.push_back(mesh);
    dispatch_on_enqueue = true;
    Canella::Logger::Debug("%d drawables", m_drawables.size());
}

#if RENDER_EDITOR_LAYOUT
std::vector<Canella::TimeQueries *> VulkanRender::get_render_graph_timers()
{
    return render_graph.time_queries;
}
#endif
