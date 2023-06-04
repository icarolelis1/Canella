#include "EditorComponents/EditorComponents.h"
#include "Window/Window.h"

using namespace Canella;


void CameraEditor::on_start() {
    //Get windo width and height
    auto window = Canella::GlfwWindow::get_instance();
    auto extent = window->getExtent();
    //Initialize camera_component projection matrix usign window width and height
    camera_component->projection = glm::perspective(glm::radians(45.0f),
                                                    float(extent.width)/ extent.height,
                                                    .1f, 100.f);
    //Attach Window Resize Event Callback.
    //Reconstruct the projection matrix when the window resizes
    std::function<void(Extent)> resize_callback = [&](Extent extent)
    {
        Canella::Logger::Info("Resizing Window");
        if(extent.width == 0 || extent.height == 0)return;
            camera_component->projection = glm::perspective(glm::radians(45.0f),
                                                                    float(extent.width)/ extent.height,
                                                                    .1f, 100.f);
    };
    //Create the event handler and register
    Event_Handler<Extent> resize_handler(resize_callback);
    window->OnWindowResize += resize_handler;
}

/*
 * Updates the main camera editor
 */
void CameraEditor::on_update(float deta_time){
    auto& camera_position = camera_component->position;
    auto& camera_euler =  camera_component->euler;

    //TODO FIX THE ENTIRE INPlUT API THIS LOOKS HORRIBLE MY GOD
    auto KeyBoard = KeyBoard::getKeyBoard();
    if(KeyBoard.getKeyPressed(GLFW_KEY_W))
    {
       camera_position += camera_euler.front *speed* deta_time;
    }
    if(KeyBoard.getKeyPressed(GLFW_KEY_S))
    {
        camera_position -= camera_euler.front *speed* deta_time;
    }
    if(KeyBoard.getKeyPressed(GLFW_KEY_A))
    {
        camera_position -= camera_euler.right *speed* deta_time;
    }
    if(KeyBoard.getKeyPressed(GLFW_KEY_D))
    {
        camera_position += camera_euler.right *speed* deta_time;
    }
    if(KeyBoard.getKeyPressed(GLFW_KEY_SPACE))
    {
        camera_position += camera_euler.up *speed* deta_time;
    }
    if(KeyBoard.getKeyPressed(GLFW_KEY_C))
    {
        camera_position -= camera_euler.up *speed* deta_time;
    }
    update_euler_directions();
}

void CameraEditor::update_euler_directions()
{
    for(auto i = 0 ; i < 1 ; i ++)
    {
        auto& camera_position = camera_component->position;
        auto& camera_euler =  camera_component->euler;
        glm::vec3 front;
        front.x = cos(glm::radians(YAW)) * cos(glm::radians(PITCH));
        front.y = sin(glm::radians(PITCH));
        front.z = sin(glm::radians(YAW)) * cos(glm::radians(PITCH));
        camera_component->euler.front = glm::normalize(front);
        glm::vec3 WorldUp = glm::vec3(0, 0, -1);
        camera_component->euler.right = glm::normalize(glm::cross(camera_euler.front, WorldUp));
        camera_component->euler.up = glm::normalize(glm::cross(camera_euler.right,camera_euler.front));
        camera_component->viewProjection =  camera_component->projection*
                                            glm::lookAt(camera_position, camera_position + camera_euler.front, camera_euler.up);
    }
}