#include "VulkanRender/VulkanRender.h"
#include "Windowing.h"
#include "Logger/Logger.hpp"
#include "Window/Window.h"
#include <vector>
using namespace Canella::RenderSystem::VulkanBackend;

void VulkanRender::initVulkanInstance()
{
	VulkanBackend::DebugLayers debugger;
	instance = new VulkanBackend::Instance(debugger, true);
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	debugger.setDebugerMessenger(createInfo, instance->handle);
}

void VulkanRender::initialize(Windowing *window)
{
}

void VulkanRender::render()
{
	FrameData &frameData = frames[current_frame];
	vkWaitForFences(device.getLogicalDevice(), 1, &frameData.imageAvaibleFence, VK_FALSE, UINT64_MAX);
	uint32_t nextImageIndex;

	VkResult result = vkAcquireNextImageKHR(device.getLogicalDevice(), swapChain.getSwapChainHandle(), UINT64_MAX, frameData.imageAcquiredSemaphore, VK_NULL_HANDLE, &nextImageIndex);
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

VulkanRender::VulkanRender(nlohmann::json &configJson, Windowing *window)
{

	initVulkanInstance();
	static_cast<GlfwWindow *>(window)->getSurface(instance->handle, &surface);
	Extent extent = static_cast<GlfwWindow *>(window)->getExtent();
	device.prepareDevice(surface, *instance);
	swapChain.prepareSwapchain(extent.width, extent.height, device, surface, VK_FORMAT_B8G8R8A8_UNORM,
							   static_cast<GlfwWindow *>(window)->getHandle(),
							   device.getQueueSharingMode());

	renderpassManager = std::make_unique<RenderpassManager>(&device, &swapChain, configJson["RenderPath"].get<std::string>().c_str());
	initDescriptorPool();
	cachePipelines();
	loadFrames();
	Canella::Logger::Info("Vulkan Renderer initialized\n");
}

void Canella::RenderSystem::VulkanBackend::VulkanRender::initDescriptorPool()
{
	descriptorPool.build(device);
}

void VulkanRender::loadFrames()
{
	auto number_of_images = swapChain.getNumberOfImages();
	for (auto i = 0; i < number_of_images; ++i)
	{
		frames.emplace_back();
		frames[i].build(&device);
	}
}

void Canella::RenderSystem::VulkanBackend::VulkanRender::cachePipelines()
{
	ShaderBindingResource viewProjection{};
	viewProjection.binding = static_cast<uint32_t>(0);
	viewProjection.size = sizeof(ViewProjection);
	viewProjection.stages = VK_SHADER_STAGE_MESH_BIT_EXT;
	viewProjection.type = ShaderResourceType::UNIFORM_BUFFER;

	std::vector<ShaderBindingResource> resources = {viewProjection};
	DescriptorSetLayout descLayout = DescriptorSetLayout();
	descLayout.build(&device, resources);

	global_descriptors.resize(swapChain.getNumberOfImages());
	descriptorPool.AllocateDescriptorSet(device, descLayout, global_descriptors);
	descLayout.destroy(&device);
}

void VulkanRender::recordCommandIndex(VkCommandBuffer &commandBuffer, uint32_t index)
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

	frames[index].commandPool.beginCommandBuffer(commandBuffer);

	auto t = renderpassManager->renderpasses["basic"];
	t->beginRenderPass(commandBuffer, clearValues, current_frame);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &rect);

	renderpassManager->renderpasses["basic"]->endRenderPass(commandBuffer);
	frames[index].commandPool.endCommandBuffer(commandBuffer);
}

VulkanRender::~VulkanRender()
{
	free(instance);

	for (auto &frame : frames)
		frame.destroy();

	swapChain.destroySwapchain(device);
	device.destroyDevice();
}