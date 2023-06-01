#include "Editor/Editor.h"
#include "CanellaUtility/CanellaUtility.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

Canella::Logger::Priority Canella::Logger::log_priority = Canella::Logger::Priority::Error_LOG;
std::mutex Canella::Logger::logger_mutex;

using namespace Canella::RenderSystem::VulkanBackend ;

Canella::Editor::Editor():application(&window,&render) {
#ifdef USE_BASE_SAMPLE
    std::fstream f(BASE_CONFIG_FILE);
    //Loads the project metadata
    nlohmann::json config; f >> config;
    //Initialize window
    window.initialize(config["Window"]);
    //set the renderer window
    render.set_windowing(&window);
    //Pass the metadata configuration for the render to load all the ressources (renderpass/pipelines...)
    render.build(config["Render"]);
    //Loads the application scenes and systems
    application.load(config);
    //Setup ImGui codee
    setup_imgui();
#endif
}

void Canella::Editor::run_editor() {
    //Starts game Loop
    application.run();
    //closes the app
    application.close();
}

void Canella::Editor::play() {
    OnStartPlay.invoke(*this);
    edit_mode = false;
}

void Canella::Editor::stop() {

    OnStopPlayEvent.invoke(*this);
    edit_mode = false;
}

void Canella::Editor::setup_imgui() {

    VkDescriptorPoolSize pool_sizes[] =
            {
                    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VK_CHECK(vkCreateDescriptorPool(render.device.getLogicalDevice(),
                                    &pool_info,
                                    nullptr,
                                    &imguiPool),
                                    "Failed to create Imgui DescriptorPool");


    //this initializes the core structures of imgui
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForVulkan(window.m_window, true);

    //this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = render.instance->handle;
    init_info.PhysicalDevice = render.device.getPhysicalDevice();
    init_info.Device = render.device.getLogicalDevice();
    init_info.Queue = render.device.getGraphicsQueueHandle();
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = static_cast<uint32_t>(render.swapChain.getNumberOfImages());
    init_info.ImageCount = static_cast<uint32_t>(render.swapChain.getNumberOfImages());
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, render.renderpassManager.renderpasses["imgui"]->get_vk_render_pass());
    auto cmd = render.request_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    render.begin_command_buffer(cmd);
    ImGui_ImplVulkan_CreateFontsTexture(cmd);
    render.end_command_buffer(cmd);
    //clear font textures from cpu data
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    std::function<void(VkCommandBuffer&,uint32_t,FrameData&)> render_editor = [=]
            (VkCommandBuffer& cmd,
            uint32_t image_index,
             FrameData& frame)
    {
        render_editor_gui(cmd,image_index,frame);
    };
    render.OnRecordCommandEvent += Event_Handler(render_editor);
}

Canella::Editor::~Editor() {
    vkDeviceWaitIdle(render.device.getLogicalDevice());
    vkDestroyDescriptorPool(render.device.getLogicalDevice(),imguiPool,nullptr);
    ImGui_ImplVulkan_Shutdown();
    render.destroy();

}

void Canella::Editor::render_editor_gui(VkCommandBuffer& command_buffer,uint32_t image_index,FrameData& current_frame) {

    auto& render_passes = render.renderpassManager.renderpasses;
    auto& swapchain = render.swapChain;
    auto& pipelin = render.swapChain;

    std::vector<VkClearValue> clearValues(1);
    clearValues[0].color = { .0f,.0f, .0f, 0.f };
    const VkViewport viewport = swapchain.get_view_port();
    const VkRect2D rect_2d = swapchain.get_rect2d();

    current_frame.commandPool.begin_command_buffer(&render.device, command_buffer,true);
    render_passes["imgui"]->beginRenderPass(command_buffer,clearValues,image_index);
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &rect_2d);

    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Canella");
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

    if (ImGui::BeginTabBar("Inspector", tab_bar_flags))
    {

    }
    ImGui::EndTabBar();
    ImGui::End();
    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
    render_passes["imgui"]->endRenderPass(command_buffer);
    current_frame.commandPool.endCommandBuffer(command_buffer);
}


