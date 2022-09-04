#include "VulkanRender/VulkanRender.h"
#include "Windowing.h"
#include "Logger/Logger.hpp"
#include "Window/Window.h"

using namespace Canella::RenderSystem::VulkanBackend;

void VulkanRender::initVulkanInstance() {
	VulkanBackend::DebugLayers debugger;
	instance = new VulkanBackend::Instance(debugger, true);
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	debugger.setDebugerMessenger(createInfo, instance->handle);

}

void VulkanRender::initialize(Windowing* window) {

	initVulkanInstance();

	static_cast<GlfwWindow*>(window)->getSurface(instance->handle, &surface);
	Extent extent = static_cast<GlfwWindow*>(window)->getExtent();
	device.prepareDevice(surface, *instance);
	swapChain.prepareSwapchain(extent.width, extent.height, device, surface, VK_FORMAT_B8G8R8A8_UNORM, static_cast<GlfwWindow*>(window)->getHandle(),device.getQueueSharingMode());

	
	Canella::Logger::Info("Vulkan Renderer initialized\n");
}

void VulkanRender::render() {
}

void VulkanRender::update(float time) {
}

VulkanRender::VulkanRender(nlohmann::json& config) {
	Canella::Logger::Debug("Render created");
}

VulkanRender::~VulkanRender() {
	free(instance);

	swapChain.destroySwapchain(device);
	device.destroyDevice();

}