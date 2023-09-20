#include "Editor/Editor.h"
#include "CanellaUtility/CanellaUtility.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"
#include "memory"
#include "Components/Components.h"
#include <random>
#include <ImGuizmo.h>
#include "Editor/Inspector.h"
#include <glm/gtc/quaternion.hpp>
#include "Mesh/Mesh.h"
#include "AssetSystem/AssetSystem.h"

Canella::Logger::Priority Canella::Logger::log_priority = Canella::Logger::Priority::Error_LOG;
std::mutex Canella::Logger::logger_mutex;

using namespace Canella::RenderSystem::VulkanBackend;

Canella::Editor::Editor() :on_select_entity(layer.on_select_entity)
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
    render.build(config["Render"],&out_put_stats);
    // Loads the application scenes and systems
    application = std::make_unique<Application>(&window, &render);
    //load data from metada
    application->load(config);
    // Setup ImGui codee
#if RENDER_EDITOR_LAYOUT
    setup_imgui();
    bind_shortcuts();
    layer.setup_layer(application.get(),on_select_operation);
#endif

    std::function<void(std::weak_ptr<Entity>)> on_select = [=](std::weak_ptr<Entity> entity){ selected_entity = entity;};
    on_select_entity += on_select;
}

void Canella::Editor::bind_shortcuts()
{
    auto &keyboard = KeyBoard::instance();
    std::function<void(int, InputAction)> short_cuts = [=](int key, InputAction action)
    {
        if (key == GLFW_KEY_H && action == InputAction::PRESS)
            show_inspector = !show_inspector;
        if (key == GLFW_KEY_2 && action == InputAction::PRESS)
            show_status = !show_status;
        if (key == GLFW_KEY_B && action == InputAction::PRESS)
            show_volume = !show_volume;

        if( key == GLFW_KEY_R && action == InputAction::PRESS)
            on_select_operation.invoke(ImGuizmo::ROTATE);

        if( key == GLFW_KEY_G && action == InputAction::PRESS)
            on_select_operation.invoke(ImGuizmo::TRANSLATE);

        if( key == GLFW_KEY_Z && action == InputAction::PRESS)
            on_select_operation.invoke(ImGuizmo::SCALE);
        if( key == GLFW_KEY_F && action == InputAction::PRESS)
        {
            if(selected_entity.expired())
                    return;
            auto target = selected_entity.lock()->get_component<TransformComponent>().position;
            auto cam = application->scene->main_camera;
            auto direction = glm::normalize( -
                     cam->entity_transform->position);
            auto quat = glm::quatLookAt(direction,cam->euler.up);
            auto delta = -cam->entity_transform->orientation + quat;
            cam->entity_transform->orientation *= delta;

        }

        if( key == GLFW_KEY_O && action == InputAction::PRESS)
        {
            auto& asset_system = Canella::AssetSystem::instance();
            TextureSlot slot = {"normal_flip_3.jpg","Albedo"};
            MaterialDescription material_description = {};
            material_description.texture_slots.push_back(std::move(slot));
            material_description.name = "BocetinhaDeCoco";
            asset_system.load_material_async( material_description,application->material_collection);
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
        if (game_mode) application->run();
        window.update();
        render.render(application->scene->main_camera->view, application->scene->main_camera->projection);
        if (KeyBoard::instance().getKeyPressed(GLFW_KEY_ESCAPE))
            break;
    }
}

void Canella::Editor::play()
{
    OnStartPlay.invoke(*this);
}

void Canella::Editor::stop()
{
    OnStopPlayEvent.invoke(*this);
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
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
        };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.pPoolSizes = pool_sizes;
    pool_info.poolSizeCount = std::size(pool_sizes);

    VK_CHECK(vkCreateDescriptorPool(render.device.getLogicalDevice(),
                                    &pool_info,
                                    nullptr,
                                    &imguiPool),"Failed to create Imgui DescriptorPool");

    IMGUI_CHECKVERSION();
    // this initializes the core structures of imgui
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    // Add the custom font
    io.Fonts->AddFontFromFileTTF(R"(resources\Fonts\RobotoRegular-3m4L.ttf)",30);

    // Setup Dear ImGui style
    ImGuiStyle &style = ImGui ::GetStyle();
    style.WindowBorderSize = 7;
    style.WindowRounding = 10;
    style.Colors[ImGuiCol_WindowBg] = MAIN_BG;
    style.Colors[ImGuiCol_TitleBg] = TITLE_BG;
    style.Colors[ImGuiCol_Header] = ImColor(133, 133, 133);
    style.Colors[ImGuiCol_Border] = BLUE;
    style.Colors[ImGuiCol_Text] = FONT_COLOR;
    style.Colors[ImGuiCol_Separator] = BLUE;
    style.Colors[ImGuiCol_ChildBg] = ImColor(0, 0, 0);
    style.Colors[ImGuiCol_Tab] = ImColor(255, 0, 0);
    style.Colors[ImGuiCol_TabActive] = ImColor(79, 53, 645);
    style.Colors[ImGuiCol_TabHovered] = ImColor(225, 0, 0);
    style.Colors[ImGuiCol_FrameBg] = MAIN_BG;
    style.Colors[ImGuiCol_TitleBgActive] = ImColor(79, 53, 64);
    style.Colors[ImGuiCol_MenuBarBg] = MENU_BG;
    style.Colors[ImGuiCol_Separator] = ImColor(211,211,211);
    style.SeparatorTextBorderSize = 10;
    style.FramePadding = ImVec2(3, 3);
    style.ChildRounding = 5;
    auto &window = GlfwWindow::get_instance();
    ImGui_ImplGlfw_InitForVulkan(window.m_window, true);

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = render.instance->handle;
    init_info.PhysicalDevice = render.device.getPhysicalDevice();
    init_info.Device = render.device.getLogicalDevice();
    init_info.Queue = render.device.getTransferQueueHandle();
    init_info.DescriptorPool = imguiPool;
    init_info.MinImageCount = static_cast<uint32_t>(render.swapChain.get_number_of_images());
    init_info.ImageCount = static_cast<uint32_t>(render.swapChain.get_number_of_images());
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
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

    std::function<void(VkCommandBuffer &, uint32_t&)> render_editor = [=](VkCommandBuffer &cmd,uint32_t image_index)
    {
        render_editor_gui(cmd, image_index);
    };
    render.OnRecordCommandEvent += Event_Handler(render_editor);
}

void Canella::Editor::render_editor_gui(VkCommandBuffer &command_buffer, uint32_t image_index)
{
    auto &render_passes = render.renderpassManager.renderpasses;
    auto &swapchain = render.swapChain;

    std ::vector<VkClearValue> clearValues;
    const VkViewport viewport = swapchain.get_view_port();
    const VkRect2D rect_2d = swapchain.get_rect2d();
    //current_frame.secondaryPool.begin_command_buffer(&render.device, command_buffer true);
    render_passes["imgui"]->beginRenderPass(command_buffer, clearValues, image_index);
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &rect_2d);
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplVulkan_NewFrame();

    ImGui ::NewFrame();
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImVec2 size = ImGui::GetContentRegionAvail();
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImGuizmo::SetRect(cursorPos.x,cursorPos.y, size.x, size.y);

    if(show_inspector)
    layer.draw_layer();

    display_graphics_status();
    display_bounding_boxes();

    ImGui ::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
    render_passes["imgui"]->endRenderPass(command_buffer);
}

void Canella::Editor::display_graphics_status()
{
    if (show_status) {


        ImGuiIO &io = ImGui::GetIO();
        float display_width = (float) io.DisplaySize.x;

        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove|
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

        ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 0xff, 0xff, 0xff, 0xff ));
        ImGui::PopStyleColor();
        ImGui::Begin( "Canella",NULL,window_flags ) ;
        ImGui::SetWindowPos(ImVec2( display_width - 360, 0));
        ImGui::Text( "FPS %.2f", double( 1.0 / application->frame_time ) * 2000 );
        ImGui::Text("Frame Time %f (ms)",double(application->frame_time/2.0));
        out_put_stats.invoke();
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

void Canella::Editor::display_bounding_boxes() {
    if( show_volume )
    {
        auto &window = GlfwWindow::get_instance();
        auto extent = window.getExtent();
        auto& view =    application->scene->main_camera->view;
        auto&projection =    application->scene->main_camera->projection;
        auto& drawables  = application->render->get_drawables();
        for(auto& drawable : drawables)
            for(auto i = 0; i < drawable.instance_count; ++i)
                for(auto& mesh : drawable.meshes){
                    SphereBoundingVolume sphere = mesh.bounding_volume;
                    auto m = *drawable.model_matrix;
                    auto s = sphere.center;
                    //sphere.center  = mesh.bounding_volume.center + drawable.instance_data[i].position_offset;
                    sphere.center  = m *  (glm::vec4(drawable.instance_data[i].position_offset.x,drawable.instance_data[i].position_offset.y,drawable.instance_data[i].position_offset.z,1.0));
                    sphere.center += s/2.0f;
                    auto  box_min_max = MeshProcessing::project_box_from_sphere(drawable.model_matrix,sphere,extent.width,extent.height,view,projection);
                    auto  box_min = ImVec2(box_min_max[0].x,box_min_max[0].y);
                    auto  box_max = ImVec2(box_min_max[1].x,box_min_max[1].y);
                    ImGui::GetBackgroundDrawList()->AddRect(box_min,box_max,IM_COL32(255, 255, 0, 255));
                }
    }
}
