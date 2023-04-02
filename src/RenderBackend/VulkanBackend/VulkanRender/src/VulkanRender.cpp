#include "VulkanRender/VulkanRender.h"

using namespace Canella::RenderSystem::VulkanBackend;

VulkanRender::VulkanRender(nlohmann::json& config, Windowing* window)
{
    initVulkanInstance();
    dynamic_cast<GlfwWindow*>(window)->getSurface(instance->handle, &surface);
    const auto [width, height] = dynamic_cast<GlfwWindow*>(window)->getExtent();
    device.prepareDevice(surface, *instance);
    swapChain.prepareSwapchain(width, height, device, surface, VK_FORMAT_B8G8R8A8_UNORM,
                               dynamic_cast<GlfwWindow*>(window)->getHandle(),
                               // ReSharper disable once CppMsExtBindingRValueToLvalueReference
                               device.getQueueSharingMode());

    renderpassManager = std::make_unique<RenderpassManager>(&device, &swapChain,
                                                            config["RenderPath"].get<std::string>().c_str());
    init_descriptor_pool();
    cache_pipelines(config["Pipelines"].get<std::string>().c_str());
    setup_frames();

    //vkCmdDrawMeshTasksEXT =  reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(device.getLogicalDevice(), "vkCmdDrawMeshTasksEXT"));
    Logger::Info("Vulkan Renderer initialized\n");
}

void VulkanRender::initVulkanInstance()
{
    DebugLayers debugger;
    instance = new Instance(debugger, true);
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    debugger.setDebugerMessenger(createInfo, instance->handle);
}

void VulkanRender::render()
{
    FrameData& frame_data = frames[current_frame];
    vkWaitForFences(device.getLogicalDevice(), 1, &frame_data.imageAvaibleFence, VK_FALSE, UINT64_MAX);
    uint32_t next_image_index;
    VkResult result = vkAcquireNextImageKHR(device.getLogicalDevice(), swapChain.getSwapChainHandle(), UINT64_MAX,
                                            frame_data.imageAcquiredSemaphore, VK_NULL_HANDLE, &next_image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // todo recreate
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error("failed to acquire swap chain image!");

    vkResetFences(device.getLogicalDevice(), 1, &frame_data.imageAvaibleFence);
    VkSubmitInfo submitInfo = {};
    record_command_index(frame_data.commandBuffer, current_frame);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    const VkSemaphore wait_semaphores[] = {frame_data.imageAcquiredSemaphore};
    constexpr VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = wait_semaphores;
    submitInfo.pWaitDstStageMask = wait_stages;

    const VkCommandBuffer cmd = frame_data.commandBuffer;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    const VkSemaphore signal_semaphores[] = {frame_data.renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signal_semaphores;

    vkQueueSubmit(device.getGraphicsQueueHandle(), 1, &submitInfo, frame_data.imageAvaibleFence);

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
    render();
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

void VulkanRender::cacheDescriptorSetLayout(
    nlohmann::json pipelineData,
    std::vector<std::shared_ptr<DescriptorSetLayout>>&
    descriptor_set_layouts,
    const int i)
{
    descriptor_set_layouts[i] = PipelineBuilder::createDescriptorSetLayout(&device,
                                                                           pipelineData["PipelineData"][
                                                                               "DescriptorSetLayouts"][i]);
    if (const char* key = pipelineData["PipelineData"]["DescriptorSetLayouts"][i]["Key"].get<std::string>().c_str();
        cachedDescriptorSetLayouts.find(key) == cachedDescriptorSetLayouts.end())
        cachedDescriptorSetLayouts[key] = std::move(descriptor_set_layouts[i]);
}

void VulkanRender::cache_pipelines(const char* pipelines)
{
    std::fstream f_stream(pipelines);
    nlohmann::json pipeline_data;
    f_stream >> pipeline_data;

    PipelineBuilder::cachePipelineData(&device, pipeline_data, cachedDescriptorSetLayouts,
                                       cachedPipelineLayouts,renderpassManager->renderpasses, cachedPipelines);
}


void VulkanRender::cacheDescriptorSetLayouts(nlohmann::json pipelineData,
                                             std::vector<VkPushConstantRange>& pushConstants)
{
    const auto descriptor_set_layout_count = pipelineData["PipelineData"]["DescriptorSetLayoutCounts"].get<
        std::int32_t>();
    std::vector<std::shared_ptr<DescriptorSetLayout>> descriptor_set_layouts(descriptor_set_layout_count);

    for (auto i = 0; i < descriptor_set_layout_count; i++)
        cacheDescriptorSetLayout(pipelineData, descriptor_set_layouts, i);
}


void VulkanRender::writeBuffers()
{
    const auto number_of_images = swapChain.getNumberOfImages();
    global_buffers.resize(number_of_images);
}

void VulkanRender::record_command_index(VkCommandBuffer& commandBuffer, uint32_t index)
{
    std::vector<VkClearValue> clear_values = {};
    clear_values.resize(1);
    clear_values[0].color = {1.0, 1.0, 1.0, .0};
    const auto [width, height] = swapChain.getExtent();
    VkViewport viewport = {};

    viewport.width = static_cast<uint32_t>(width);
    viewport.height = static_cast<uint32_t>(height);

    viewport.maxDepth = 1.0f;

    VkRect2D rect = {};
    rect.extent.width = static_cast<uint32_t>(width);
    rect.extent.height = static_cast<uint32_t>(height);
    rect.offset = {0, 0};

    frames[index].commandPool.beginCommandBuffer(&device, commandBuffer, true);


    auto render_pass = renderpassManager->renderpasses["basic"];
    render_pass->beginRenderPass(commandBuffer, clear_values, current_frame);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, cachedPipelines["MeshShaderPipeline"]->getPipelineHandle());
    vkCmdDrawMeshTasksEXT(commandBuffer, 1, 1, 1);        
    
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &rect);

    renderpassManager->renderpasses["basic"]->endRenderPass(commandBuffer);
    frames[index].commandPool.endCommandBuffer(commandBuffer);
}


VulkanRender::~VulkanRender()
{
    free(instance);

    for (auto& frame : frames)
        frame.destroy();

    swapChain.destroySwapchain(device);
    device.destroyDevice();
    destroyDescriptorSetLayouts();
    destroyPipelineLayouts();
}

void VulkanRender::destroyPipelineLayouts()
{
    const auto it = cachedDescriptorSetLayouts.begin();
    while (it != cachedDescriptorSetLayouts.end())
        it->second->destroy(&device);
}

void VulkanRender::destroyDescriptorSetLayouts()
{
    const auto it = cachedPipelineLayouts.begin();
    while (it != cachedPipelineLayouts.end())
        it->second->destroy(&device);
}
