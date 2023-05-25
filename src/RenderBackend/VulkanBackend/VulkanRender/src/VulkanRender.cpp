#include "VulkanRender/VulkanRender.h"

#include <core/compressed_pair.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

using namespace Canella::RenderSystem::VulkanBackend;

/**
 * \brief Creates the VulkanRender
 * \param config configuration file for the render
 * \param window window that's going to be rendered into
 */
VulkanRender::VulkanRender(nlohmann::json& config, Windowing* window)
    : resources_manager(&this->device) {
    init_vulkan_instance();
    //drawables = std::move(_drawables);
    dynamic_cast<GlfwWindow*>(window)->getSurface(instance->handle, &surface);
    const auto [width, height] = dynamic_cast<GlfwWindow*>(window)->getExtent();

    device.prepareDevice(surface, *instance);

    swapChain.prepare_swapchain(width,
                                height,
                                device,
                                surface,
                                VK_FORMAT_B8G8R8A8_UNORM,
                                dynamic_cast<GlfwWindow *>(window)->getHandle(),
                                device.getQueueSharingMode());

    renderpassManager = std::make_unique<RenderpassManager>(&device, &swapChain,
                                                            config["RenderPath"].get<std::string>().c_str());

    init_descriptor_pool();
    cache_pipelines(config["Pipelines"].get<std::string>().c_str());
    setup_frames();
    allocate_global_usage_buffers();
    allocate_global_descriptorsets();
    write_global_descriptorsets();
    render_graph.load_render_graph(config["RenderGraph"].get<std::string>().c_str(),this);

    vkCmdDrawMeshTasksEXT = reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(
            device.getLogicalDevice(),
            "vkCmdDrawMeshTasksEXT"));
    
    transfer_pool.build(&device,
                        POOL_TYPE::TRANSFER,
                        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
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

void VulkanRender::enqueue_drawables(Drawables& drawables)
{
    m_drawables = drawables;
    render_graph.load_resources(this);
}

void VulkanRender::render(glm::mat4& _view_projection)
{
    FrameData& frame_data = frames[current_frame];
    vkWaitForFences(device.getLogicalDevice(), 1, &frame_data.imageAvaibleFence, VK_FALSE, UINT64_MAX);
    uint32_t next_image_index;
    const auto eye_pos = glm::vec3(0.0 , 3.1, -12);
    t += 0.01;
    if(t >= 1000000.00)
        t = 0.0;
    ViewProjection view_projection{};
    view_projection.model = glm::mat4(1.0f);
    view_projection.model = glm::rotate(view_projection.model,glm::radians(-90.0f),glm::vec3(1,0,0));
    view_projection.model = glm::rotate(view_projection.model,glm::radians(t),glm::vec3(0,0,1));
    view_projection.projection = glm::perspective(glm::radians(45.0f), 1.0f, .1f, 100.f);
    view_projection.view = glm::lookAt(eye_pos, glm::vec3(0, 1, 0), glm::vec3(0, -1, 0));

    global_buffers[current_frame]->udpate(view_projection);
    VkResult result = vkAcquireNextImageKHR(device.getLogicalDevice(),
                                            swapChain.getSwapChainHandle(),
                                            UINT64_MAX,
                                            frame_data.imageAcquiredSemaphore,
                                            VK_NULL_HANDLE,
                                            &next_image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // todo recreate
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swap chain image!");

    vkResetFences(device.getLogicalDevice(), 1, &frame_data.imageAvaibleFence);
    VkSubmitInfo submit_info = {};

    record_command_index(frame_data.commandBuffer, _view_projection, current_frame);

    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    const VkSemaphore wait_semaphores[] = {frame_data.imageAcquiredSemaphore};
    constexpr VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    const VkCommandBuffer cmd = frame_data.commandBuffer;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd;
    const VkSemaphore signal_semaphores[] = {frame_data.renderFinishedSemaphore};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    vkQueueSubmit(device.getGraphicsQueueHandle(), 1, &submit_info, frame_data.imageAvaibleFence);
    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;
    const VkSwapchainKHR swap_chains[] = {swapChain.getSwapChainHandle()};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &next_image_index;
    result = vkQueuePresentKHR(device.getGraphicsQueueHandle(), &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        // recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
    current_frame = (current_frame + 1) % 3;
}

void VulkanRender::update(float time)
{
}

void VulkanRender::init_descriptor_pool()
{
    descriptorPool.build(device);
}

void VulkanRender::setup_frames()
{
    const auto number_of_images = swapChain.getNumberOfImages();
    for (uint32_t i = 0; i < number_of_images; ++i)
    {
        frames.emplace_back();
        frames[i].build(&device);
    }
}

void VulkanRender::cache_pipelines(const char* pipelines)
{
    std::fstream f_stream(pipelines);
    nlohmann::json pipeline_data;
    f_stream >> pipeline_data;
    std::vector<VkPushConstantRange> pushConstants;
    PipelineBuilder::cache_pipeline_data(&device, pipeline_data, cachedDescriptorSetLayouts,
                                         cachedPipelineLayouts, renderpassManager->renderpasses, cachedPipelines);
}

void VulkanRender::allocate_global_usage_buffers()
{
    const auto number_of_images = swapChain.getNumberOfImages();
    for (auto i = 0; i < number_of_images; i++)
        global_buffers.push_back(std::make_shared<Buffer>(&device, sizeof(ViewProjection),
                                                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT|
                                                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));
}

void VulkanRender::write_global_descriptorsets()
{
    auto i = 0;
    for (auto& buffer : global_buffers)
    {
        std::vector<VkDescriptorBufferInfo> buffer_infos;
        std::vector<VkDescriptorImageInfo> image_infos;
        buffer_infos.resize(1);
        buffer_infos[0].buffer = buffer->getBufferHandle();
        buffer_infos[0].offset = static_cast<uint32_t>(0);
        buffer_infos[0].range = sizeof(ViewProjection);
        DescriptorSet::update_descriptorset(&device, global_descriptors[i], buffer_infos, image_infos, false);
        i++;
    }
}

void VulkanRender::record_command_index(VkCommandBuffer& commandBuffer,
                                        glm::mat4& viewProjection,
                                        uint32_t index)
{

    frames[index].commandPool.beginCommandBuffer(&device, commandBuffer, true);
    render_graph.execute(commandBuffer,this,index);
    frames[index].commandPool.endCommandBuffer(commandBuffer);
}

void VulkanRender::allocate_global_descriptorsets()
{
    global_descriptors.resize(swapChain.getNumberOfImages());
    for (auto& global_descriptor : global_descriptors)
        descriptorPool.allocate_descriptor_set(
            device,
            cachedDescriptorSetLayouts["ViewProjection"],
            global_descriptor);
}

VulkanRender::~VulkanRender()
{
    free(instance);
    for (auto& frame : frames)
        frame.destroy();

    swapChain.destroySwapchain(device);
    device.destroyDevice();
    destroy_descriptor_set_layouts();
    destroy_pipeline_layouts();
    transfer_pool.destroy(&device);
    //T0DO DESTROY BUFFER
    //TODO REMOVE THE BUFFER ALOCATION FROM VULKAN_RENDER

}

void VulkanRender::destroy_pipeline_layouts()
{
    const auto it = cachedDescriptorSetLayouts.begin();
    while (it != cachedDescriptorSetLayouts.end())
        it->second->destroy(&device);
}

void VulkanRender::destroy_descriptor_set_layouts()
{
    const auto it = cachedPipelineLayouts.begin();
    while (it != cachedPipelineLayouts.end())
        it->second->destroy(&device);
}

Canella::Drawables &VulkanRender::get_drawables() {
    return m_drawables;
}
