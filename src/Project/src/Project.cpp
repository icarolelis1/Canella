#include "Project/Project.h"
#include "Components/Components.h"
#include "Systems/Systems.h"
using namespace Canella;

/**
 * \brief Load a project from disk
 * \param config project File
 */
void Application::load(nlohmann::json& config)
{
    JobSystem::initialize();
    //Setup Project Folder
    setup_project_folder(config);
    //init windowing
    //window.initialize(config["Window"]);
    //load scene
    scene = std::make_shared<Scene>();
    //serialize scene
    serializer.Serialize(scene, config["Scenes"]);
    auto e1 =  scene->CreateEntity();
    e1.add_custom_component().bind<ScriptTest>();
    //init render
    //render = std::make_unique<RenderSystem::VulkanBackend::VulkanRender>(config["Render"], &window);
    init_systems();
}

/**
 * \brief Create project structure folder
 * \param data configuration file
 */
void Application::setup_project_folder(nlohmann::json& data)
{
    assetsFolder = data["Assets"].get<std::string>();
}

void Application::init_systems()
{
    //Loads all the scenes in the scene before run time
    load_meshes_from_scene(assetsFolder,scene);
    std::vector<ModelMesh> meshes;
    get_meshes_on_scene(meshes,scene);
    render->enqueue_drawables(meshes);
    //Gets the reference for the main Camera
    main_camera = get_main_camera(scene);

    for (auto &[entt_value, entity]: scene->m_EntityLibrary)
        if (entity->has_component<ModelAssetComponent>())
        {
            auto& comp = entity->get_component<ModelAssetComponent>();
            auto& m = comp.mesh.meshes;
        }
}

void Application::update_systems()
{
    update_camera(main_camera,*window);

    //Update custom components
    scene->m_registry.view<Behavior>().each([=](auto entity,auto& script){
        if(!script.entity)
        {
            script.instantiate();
            script.on_create(script.entity);
        }
        script.on_update(script.entity, 0.0f);
    });


}

/**
 * \brief Enter the game loop
 */
void Application::run()
{
    while (playing) {

        playing = ~window->shouldCloseWindow();
        window->update();
        update_systems();
        render->render(main_camera.viewProjection);

        if (KeyBoard::getKeyBoard().getKeyPressed(GLFW_KEY_ESCAPE))
        {
            // serializer.Deserialize(scene);
            break;
        }
        /*for (auto &[entt_value, entity]: scene->m_EntityLibrary)
            if (entity->has_component<ModelAssetComponent>())
            {
                auto& comp = entity->get_component<ModelAssetComponent>();
                auto& m = comp.mesh.meshes;
            }
        */
    }
}

void Application::close()
{
}


Application::~Application()
{
    JobSystem::stop();
    close();

    scene->m_registry.view<Behavior>().each([=](auto entity,auto& script){
        if(!script.entity)
        {
            script.instantiate();
            script.on_create(script.entity);
        }
        script.destroy();
    });
}

Application::Application(Canella::GlfwWindow *_window, Canella::Render *_render) {
    window = _window;
    render = _render;
}
