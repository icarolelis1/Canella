#include "Editor/Editor.h"
#include "CanellaUtility/CanellaUtility.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "memory"
#include "Components/Components.h"
Canella::Logger::Priority Canella::Logger::log_priority = Canella::Logger::Priority::Error_LOG;
std::mutex Canella::Logger::logger_mutex;

using namespace Canella::RenderSystem::VulkanBackend;

Canella::Editor::Editor()
{
    std::fstream f(BASE_CONFIG_FILE);
    // Loads the project metadata
    nlohmann::json config;
    f >> config;
    // Initialize window
    auto &window = GlfwWindow::get_instance();
    window.initialize(config["Window"]);
    // set the renderer window
    render.set_windowing(&window);
    // Pass the metadata configuration for the render to load all the ressources (renderpass/pipelines...)
    render.build(config["Render"]);
    // Loads the application scenes and systems
    application = std::make_unique<Application>(&window, &render);
    //
    application->load(config);
    // Setup ImGui codee
#if RENDER_EDITOR_LAYOUT
    setup_imgui();
    // Get Time Queriries written by the render_graph
    time_queries = render.get_render_graph_timers();
#endif
    bind_shortcuts();
}

void Canella::Editor::bind_shortcuts()
{
    auto &keyboard = KeyBoard::instance();
    std::function<void(int, InputAction)> short_cuts = [=](int key, InputAction action)
    {
        Canella::Logger::Debug("%d key_code ", key);
        if (key == GLFW_KEY_F1 && action == InputAction::PRESS)
        {
            display_statistics = !display_statistics;
            Canella::Logger::Debug("%d key_code ", display_statistics);
        }

        if (key == GLFW_KEY_P && action == InputAction::PRESS)
        {
            game_mode = !game_mode;
            Canella::Logger::Debug("%d key_code ", display_statistics);
        }
    };
    keyboard.OnKeyInput += Event_Handler(short_cuts);
}

void Canella::Editor::run_editor()
{
    // Starts game Loop
    auto &window = GlfwWindow::get_instance();

    while (true)
    {
        playing = ~window.shouldCloseWindow();
        if (game_mode)
            application->run();
        window.update();
        render.render(application->scene->main_camera->view, application->scene->main_camera->projection);
        if (KeyBoard::instance().getKeyPressed(GLFW_KEY_ESCAPE))
            break;
    }

    // closes the app
    application->close();
}

void Canella::Editor::play()
{
    OnStartPlay.invoke(*this);
    scene_mode = false;
}

void Canella::Editor::stop()
{

    OnStopPlayEvent.invoke(*this);
    scene_mode = false;
}

void Canella::Editor::setup_imgui()
{

    VkDescriptorPoolSize pool_sizes[] = 
        {
            {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.pPoolSizes = pool_sizes;
    pool_info.poolSizeCount = std::size(pool_sizes);

    VK_CHECK(vkCreateDescriptorPool(render.device.getLogicalDevice(),
                                    &pool_info,
                                    nullptr,
                                    &imguiPool),
             "Failed to create Imgui DescriptorPool");

    IMGUI_CHECKVERSION();
    // this initializes the core structures of imgui
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    // Add the custom font
    io.Fonts->AddFontFromFileTTF(
        R"(resources\Fonts\RobotoRegular-3m4L.ttf)",
        30);

    // Setup Dear ImGui style
    ImGuiStyle &style                    = ImGui     ::GetStyle();
    style.WindowBorderSize               = 7;
    style.WindowRounding                 = 10;
    style.Colors[ImGuiCol_WindowBg]      = MAIN_BG;
    style.Colors[ImGuiCol_TitleBg]       = TITLE_BG;
    style.Colors[ImGuiCol_Header]        = ImColor(255, 255, 255);
    style.Colors[ImGuiCol_Border]        = BLUE;
    style.Colors[ImGuiCol_Text]          = FONT_COLOR;
    style.Colors[ImGuiCol_Separator]     = BLUE;
    style.Colors[ImGuiCol_ChildBg]       = ImColor(0, 0, 0);
    style.Colors[ImGuiCol_Tab]           = ImColor(255, 0, 0);
    style.Colors[ImGuiCol_TabActive]     = ImColor(79, 53, 645);
    style.Colors[ImGuiCol_TabHovered]    = ImColor(225, 0, 0);
    style.Colors[ImGuiCol_FrameBg]       = ImColor(255, 0, 0);
    style.Colors[ImGuiCol_TitleBgActive] = ImColor(79, 53, 64);
    style.Colors[ImGuiCol_MenuBarBg]     = MENU_BG;
    style.SeparatorTextBorderSize        = 10;
    style.FramePadding                   = ImVec2(3, 3);
    style.ChildRounding                  = 5;
    auto &window                         = GlfwWindow::get_instance();
    ImGui_ImplGlfw_InitForVulkan(window.m_window, true);

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance       = render.instance->handle;
    init_info.PhysicalDevice = render.device.getPhysicalDevice();
    init_info.Device         = render.device.getLogicalDevice();
    init_info.Queue          = render.device.getTransferQueueHandle();
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount  = static_cast<uint32_t>(render.swapChain.get_number_of_images());
    init_info.ImageCount     = static_cast<uint32_t>(render.swapChain.get_number_of_images());
    init_info.MSAASamples    = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info,
                          render.renderpassManager.renderpasses["imgui"]->get_vk_render_pass());

    auto cmd = render.request_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    render.begin_command_buffer(cmd);
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd;
    ImGui_ImplVulkan_CreateFontsTexture(cmd);
    render.end_command_buffer(cmd);
    vkQueueSubmit(render.device.getGraphicsQueueHandle(),
                  1,
                  &submit_info, VK_NULL_HANDLE);

    std::function<void(VkCommandBuffer &, uint32_t, FrameData &)> render_editor = [=](VkCommandBuffer &cmd,
                                                                                      uint32_t image_index,
                                                                                      FrameData &frame)
    {
        render_editor_gui(cmd, image_index, frame);
    };
    render.OnRecordCommandEvent += Event_Handler(render_editor);
}

void Canella::Editor::render_editor_gui(VkCommandBuffer &dsds, uint32_t image_index, FrameData &current_frame)
{

    auto &render_passes = render.renderpassManager.renderpasses;
    auto &swapchain = render.swapChain;

    std                                  ::vector<VkClearValue> clearValues(1);
    clearValues[0].color                 = {.0f, .0f, .0f, 0.f};
    const VkViewport viewport            = swapchain.get_view_port();
    const VkRect2D rect_2d               = swapchain.get_rect2d();
    current_frame.secondaryPool.begin_command_buffer(&render.device, current_frame.editor_command, true);
    render_passes["imgui"]->beginRenderPass(current_frame.editor_command, clearValues, image_index);
    vkCmdSetViewport(current_frame.editor_command, 0, 1, &viewport);
    vkCmdSetScissor(current_frame.editor_command, 0, 1, &rect_2d);
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplVulkan_NewFrame();
    ImGui                                ::NewFrame();
    editor_layout();

    ImGui                                ::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), current_frame.editor_command);
    render_passes["imgui"]->endRenderPass(current_frame.editor_command);
    current_frame.secondaryPool.endCommandBuffer(current_frame.editor_command);
}

void Canella::Editor::editor_layout()
{
    if (display_statistics)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0xff, 0xff, 0xff, 0xff));
        if (!ImGui::Begin("Canella"))
        {
            ImGui::End();
            ImGui::PopStyleColor();
            return;
        }
        ImGui::PopStyleColor();

        // if (ImGui::BeginMainMenuBar())
        // {
        //     if (ImGui::BeginMenu("Menu"))
        //     {
        //         if (ImGui::MenuItem("Quit"))
        //         {
        //             Canella::Logger::Info("Quit");
        //         }
        //         ImGui::EndMenu();
        //     }
        //     ImGui::EndMainMenuBar();
        // }
        auto i = 0;

        for (auto *timer : time_queries)
        {
            ImGui::PushID(i);
            ImGui::Text("%s %lf", timer->name.c_str(), timer->time);
            ImGui::Separator();
            ImGui::PopID();
            i++;
        }

        ImGui::End();
    }
    ImGui::ShowDemoWindow();
}

Canella::Editor::~Editor()
{
#if RENDER_EDITOR_LAYOUT
    ImGui_ImplVulkan_DestroyFontUploadObjects();
    vkDeviceWaitIdle(render.device.getLogicalDevice());
    vkDestroyDescriptorPool(render.device.getLogicalDevice(), imguiPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
#endif
    render.destroy();
}
