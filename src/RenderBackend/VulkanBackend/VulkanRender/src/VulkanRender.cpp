#include "VulkanRender/VulkanRender.h"

using namespace Canella::RenderSystem::VulkanBackend;

/**
 * \brief 
 * \param configJson 
 * \param window 
 */
VulkanRender::VulkanRender(nlohmann::json& configJson, Windowing* window)
{
    initVulkanInstance();
    static_cast<GlfwWindow*>(window)->getSurface(instance->handle, &surface);
    Extent extent = static_cast<GlfwWindow*>(window)->getExtent();
    device.prepareDevice(surface, *instance);
    swapChain.prepareSwapchain(extent.width, extent.height, device, surface, VK_FORMAT_B8G8R8A8_UNORM,
                               static_cast<GlfwWindow*>(window)->getHandle(),
                               // ReSharper disable once CppMsExtBindingRValueToLvalueReference
                               device.getQueueSharingMode());

    renderpassManager = std::make_unique<RenderpassManager>(&device, &swapChain,
                                                            configJson["RenderPath"].get<std::string>().c_str());
    initDescriptorPool();
    cachePipelines(configJson["Pipelines"].get<std::string>().c_str());
    setupFrames();
    Canella::Logger::Info("Vulkan Renderer initialized\n");
}


void VulkanRender::initVulkanInstance()
{
    VulkanBackend::DebugLayers debugger;
    instance = new VulkanBackend::Instance(debugger, true);
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    debugger.setDebugerMessenger(createInfo, instance->handle);
}

void VulkanRender::render()
{
    FrameData& frameData = frames[current_frame];
    vkWaitForFences(device.getLogicalDevice(), 1, &frameData.imageAvaibleFence, VK_FALSE, UINT64_MAX);
    uint32_t nextImageIndex;

    VkResult result = vkAcquireNextImageKHR(device.getLogicalDevice(), swapChain.getSwapChainHandle(), UINT64_MAX,
                                            frameData.imageAcquiredSemaphore, VK_NULL_HANDLE, &nextImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // todo
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    vkResetFences(device.getLogicalDevice(), 1, &frameData.imageAvaibleFence);

    VkSubmitInfo submitInfo = {};

    recordCommandIndex(frameData.commandBuffer, current_frame);
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {frameData.imageAcquiredSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    VkCommandBuffer cmd = frameData.commandBuffer;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    VkSemaphore signalSemaphores[] = {frameData.renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkResult r = vkQueueSubmit(device.getGraphicsQueueHandle(), 1, &submitInfo, frameData.imageAvaibleFence);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain.getSwapChainHandle()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &nextImageIndex;

    result = vkQueuePresentKHR(device.getGraphicsQueueHandle(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        // recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
    current_frame = (current_frame + 1) % 2;
}

void VulkanRender::update(float time)
{
    render();
}

void Canella::RenderSystem::VulkanBackend::VulkanRender::initDescriptorPool()
{
    descriptorPool.build(device);
}

void VulkanRender::setupFrames()
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
    std::vector<DescriptorSetLayout>&
    descriptor_set_layouts,
    const int i)
{
    descriptor_set_layouts[i] = PipelineBuilder::createDescriptorSetLayout(&device,
                                                                           pipelineData["PipelineData"][
                                                                               "DescriptorSetLayouts"][i]);
    const char* key = pipelineData["PipelineData"]["DescriptorSetLayouts"][i]["Key"].get<std::string>().c_str();
    if (cachedDescriptorSetLayouts.find(key) == cachedDescriptorSetLayouts.end())
        cachedDescriptorSetLayouts[key] = std::move(descriptor_set_layouts[i]);
}

// void VulkanRender::createPipelines(const char* pipelines)
// {
//     std::fstream f(pipelines);
//     nlohmann::json pipelineData;
//     f >> pipelineData;
// }

void VulkanRender::cachePipelines(const char* pipelines)
{
    std::fstream f_stream(pipelines);
    nlohmann::json pipeline_data;
    f_stream >> pipeline_data;

    PipelineBuilder::cachePipelineData(&device,pipeline_data, cachedDescriptorSetLayouts,
        cachedPipelineLayouts, cachedPipelines);
    // createPipelineLayouts(pipelines);
    // createPipelines(pipelines);
}


void VulkanRender::cacheDescriptorSetLayouts(nlohmann::json pipelineData,
                                             std::vector<VkPushConstantRange>& pushConstants)
{
    const auto descriptorSetLayoutCount = pipelineData["PipelineData"]["DescriptorSetLayoutCounts"].get<
        std::uint32_t>();
    std::vector<DescriptorSetLayout> descriptor_set_layouts(descriptorSetLayoutCount);

    for (auto i = 0; i < descriptorSetLayoutCount; i++)
        cacheDescriptorSetLayout(pipelineData, descriptor_set_layouts, i);
}

void VulkanRender::createPipelineLayouts(const char* pipelines)
{
    std::fstream fStream(pipelines);
    nlohmann::json pipelineData;
    fStream >> pipelineData;

    //Cache DescriptorSetLayouts
    std::vector<VkPushConstantRange> pushConstants;
    cacheDescriptorSetLayouts(pipelineData, pushConstants);
    nlohmann::json pipelines_json = pipelineData["PipelineData"]["Pipelines"];
    const auto number_of_pipelines = pipelineData["PipelineData"]["PipelinesCount"].get<std::uint32_t>();

    //Cache Pipeline Layouts
    for (auto i = 0; i < number_of_pipelines; i ++)
    {
        nlohmann::json pipeline = pipelines_json[i];
        auto number_of_descriptorset_layouts = pipeline["NumerOfDescriptorSets"].get<std::uint32_t>();
        std::vector<DescriptorSetLayout> descriptor_set_layouts(number_of_descriptorset_layouts);
        for (auto j = 0; j < number_of_descriptorset_layouts; j++)
            descriptor_set_layouts.push_back(
                cachedDescriptorSetLayouts[pipeline["PipelineLayoutDescriptors"][j].get<std::string>().c_str()]);
        cachedPipelineLayouts[pipeline["Key"].get<std::string>().c_str()] = PipelineLayout();
        cachedPipelineLayouts[pipeline["Key"].get<std::string>().c_str()].build(&device, descriptor_set_layouts, pushConstants);
    }


    //
    // pipelineLayouts["basic"] = PipelineLayout();
    // pipelineLayouts["basic"].build(&device, descriptor_set_layouts, pushConstants);
    //
    // global_descriptors.resize(swapChain.getNumberOfImages());
    // descriptorPool.AllocateDescriptorSet(device, descriptor_set_layouts[0], global_descriptors);
    // descriptor_set_layouts[0].destroy(&device);
}

void VulkanRender::writeBuffers()
{
    auto number_of_images = swapChain.getNumberOfImages();
    global_buffers.resize(number_of_images);
}

void VulkanRender::recordCommandIndex(VkCommandBuffer& commandBuffer, uint32_t index)
{
    std::vector<VkClearValue> clearValues = {};
    clearValues.resize(1);
    clearValues[0].color = {1.0, 1.0, 1.0, .0};
    VkExtent2D e = swapChain.getExtent();
    VkViewport viewport = {};

    viewport.width = static_cast<uint32_t>(e.width);
    viewport.height = static_cast<uint32_t>(e.height);

    viewport.maxDepth = 1.0f;

    VkRect2D rect = {};
    rect.extent.width = static_cast<uint32_t>(e.width);
    rect.extent.height = static_cast<uint32_t>(e.height);
    rect.offset = {0, 0};

    frames[index].commandPool.beginCommandBuffer(&device, commandBuffer, true);

    auto render_pass = renderpassManager->renderpasses["basic"];
    render_pass->beginRenderPass(commandBuffer, clearValues, current_frame);
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
        it->second.destroy(&device);
}

void VulkanRender::destroyDescriptorSetLayouts()
{
    const auto it = cachedPipelineLayouts.begin();
    while (it != cachedPipelineLayouts.end())
        it->second.destroy(&device);
}
