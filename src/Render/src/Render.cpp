#include "Render/Render.h"

RenderSystem::Render *RenderSystem::Render::render_instance = nullptr;
RenderSystem::Render::Render(nlohmann::json &configFile)
{
    if (configFile["Api"] == "VULKAN")
    {
        #define VULKAN_BACKEND
        VulkanBackend::DebugLayers debugger;
        VulkanBackend::Instance instance(debugger, true);
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        debugger.setDebugerMessenger(createInfo, instance.handle);
    }
};
RenderSystem::RenderConfig &RenderSystem::Render::getConfig() { return config; };
RenderSystem::Render *RenderSystem::Render::createRender(nlohmann::json &configFile)
{

    if (render_instance == nullptr)
    {
        render_instance = new RenderSystem::Render(configFile);
    };
    return render_instance;
};