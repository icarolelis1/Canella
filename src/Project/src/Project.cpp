#include "Project/Project.h"
#include "Components/Components.h"
#include "Systems/Systems.h"
using namespace Canella;


Application::Application(Canella::GlfwWindow *_window, Canella::Render *_render):application_time(0){
    window = _window;
    render = _render;
}

/**
 * \brief Load a project from disk
 * \param config project File
 */
void Application::load(nlohmann::json& config)
{
    JobSystem::initialize();
    //Setup Project Folder
    setup_project_folder(config);
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
    //Get all the meshes loaded from the scene
    std::vector<ModelMesh> meshes;
    get_meshes_on_scene(meshes,scene);
    //For static meshes the transform needs to be uploaded before enqueing the drawables
    update_transforms(scene);
    //Send the meshes to be rendered by the renderer
    render->enqueue_drawables(meshes);
    //Gets the reference for the main Camera
    main_camera = get_main_camera(scene);
    //todo Start scripts only on sceneplay when using Editor
    //starts the scripts calling on_start
    start_scripts(scene);
}

void Application::update_systems(float frame_time)
{
    update_scripts(scene,frame_time);
    //Todo calculate the model matrix in shader side not CPU.
    //Update scene transforms
    update_transforms(scene);
}

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
