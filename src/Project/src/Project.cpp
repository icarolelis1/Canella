#include "Project/Project.h"
#include "Components/Components.h"
#include "Systems/Systems.h"
using namespace Canella;

/**
 * \brief Load a project from disk
 * \param config project File
 */
void Project::load(nlohmann::json& config)
{
    JobSystem::initialize();
    //Setup Project Folder
    setup_project_folder(config);
    //init windowing
    window.initialize(config["Window"]);
    //load scene
    scene = std::make_shared<Scene>();
    //serialize scene
    serializer.Serialize(scene, config["Scenes"]);
    //init render
    render = std::make_unique<RenderSystem::VulkanBackend::VulkanRender>(config["Render"], &window);
    init_systems();
}

/**
 * \brief Create project structure folder
 * \param data configuration file
 */
void Project::setup_project_folder(nlohmann::json& data)
{
    assetsFolder = data["Assets"].get<std::string>();
}

void Project::init_systems()
{
    //Loads all the scenes in the scene before run time
    load_meshes_from_scene(assetsFolder,scene);
    std::vector<Mesh> meshes;
    get_meshes_on_scene(meshes,scene);
    render->enqueue_drawables(meshes);
    //Gets the reference for the main Camera
    main_camera = get_main_camera(scene);
}

void Project::update_systems()
{
    update_camera(main_camera,window);
    
}

/**
 * \brief Enter the game loop
 */
void Project::run()
{    
    while (playing)
    {
        playing = ~window.shouldCloseWindow();
        glfwPollEvents();
        update_systems();
        render->render(main_camera.viewProjection);
        if (KeyBoard::getKeyBoard().getKeyPressed(GLFW_KEY_ESCAPE))
        {
            // serializer.Deserialize(scene);
            break;
        }
    }
}

void Project::close()
{
    render.reset();
}

Project::~Project()
{
    JobSystem::stop();
    close();
}
