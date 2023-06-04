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

    scene->m_registry.view<Behavior>().each([=](auto entity, auto &behavior)
        {
        if (!behavior.instance)
            {
                behavior.instance = behavior.instantiate_fn();
                behavior.instance->entt_entity = entity;
            }
            behavior.instance->on_start();
        });
}

void Application::update_systems(float frame_time)
{
    //Update main Camera
    //update_camera(main_camera,*window);
    //Update Behavior scripts
    scene->m_registry.view<Behavior>().each([=](auto entity,auto& behavior)
    {
        if(!behavior.instance)
        {
            behavior.instance = behavior.instantiate_fn();
            behavior.instance->entt_entity = entity;
        }
        behavior.instance->on_update(frame_time);

    });
    //Todo calculate the model matrix in shader side not CPU.
    //Update scene transforms
    update_transforms(scene);
}

/**
 * \brief Enter the game loop
 */
void Application::run()
{
    while (playing) {

        float time = (float)glfwGetTime();
        application_time.time = time - application_time.last_time_frame;
        frame_time = application_time.time_in_milli();
        application_time.last_time_frame = time;
        playing = ~window->shouldCloseWindow();
        
        window->update();
        update_systems(frame_time);
        render->render(main_camera->viewProjection);


        if (KeyBoard::getKeyBoard().getKeyPressed(GLFW_KEY_ESCAPE))
            break;

    }
}

void Application::close()
{
}


Application::~Application()
{
    JobSystem::stop();
    close();
}

Application::Application(Canella::GlfwWindow *_window, Canella::Render *_render):application_time(0){
    window = _window;
    render = _render;
}

