#include "EditorComponents/EditorComponents.h"
#include "Window/Window.h"

using namespace Canella;


void CameraEditor::on_start() {
    //Get windo width and height
    auto& window = Canella::GlfwWindow::get_instance();
    auto extent = window.getExtent();
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
    window.OnWindowResize += resize_handler;
    //Todo try to improve this API too long
    set_mouse_callbacks();
}

/*
 * Updates the main camera editor
 */
void CameraEditor::on_update(float delta_time){
    auto& camera_position = camera_component->position;
    auto& camera_euler =  camera_component->euler;
    auto& mouse = Mouse::instance();
    auto pos = mouse.get_cursor_pos();
    auto& keyboard = KeyBoard::getKeyBoard();
    last_x = pos.x;
    last_y = pos.y;
    if (PITCH > 89.0f)
        PITCH = 89.0f;
    if (PITCH < -89.0f)
        PITCH = -89.0f;
    camera_input_keys(delta_time,camera_position,camera_euler,keyboard,pos);
    update_euler_directions();
}

void CameraEditor::camera_input_keys(float deta_time,
                                    glm::vec3 &camera_position,
                                    const Euler &camera_euler,
                                    Canella::KeyBoard &KeyBoard,
                                    Canella::CursorPos& cursor_position)
{
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
}

void CameraEditor::update_euler_directions()
{
    //FPS camera:  RotationX(pitch) * RotationY(yaw)
    glm::quat qPitch = glm::angleAxis(PITCH, glm::vec3(1, 0, 0));
    glm::quat qYaw = glm::angleAxis(YAW, glm::vec3(0, 0, 1));
    glm::quat qRoll = glm::angleAxis(ROLL,glm::vec3(0,1,0));

    //For a FPS camera we can omit roll
    orientation = qPitch * qYaw * qRoll;
    orientation = glm::normalize(orientation);
    glm::mat4 rotate = glm::mat4_cast(orientation);

    glm::mat4 translate = glm::mat4(1.0f);

    translate = glm::translate(translate, -camera_component->position);

    camera_component->euler.front = glm::normalize(glm::vec3(0.f,0.f,-1.0f)*orientation);
    camera_component->euler.right = glm::normalize(glm::vec3(1.f,0.f,0.0f)*orientation);
    camera_component->euler.up = glm::normalize(glm::vec3(0.f,1.f,0.0f)*orientation);
    camera_component->view = rotate * translate;

}

void CameraEditor::set_mouse_callbacks()
{
    //Set the mouse click callback
    std::function<void(MouseButton,MouseAction)> mouse_press = [=](MouseButton button ,MouseAction action)->void
    {
        auto& mouse = Mouse::instance();
        auto pos = mouse.get_cursor_pos();
        drag_x = pos.x;
        drag_y = pos.y;

        if( button == MouseButton::MIDDLE_MOUSE)
        {
            if(action == MouseAction::HOLD)
            dragging_position = true;

            //Handle Mouse Drag Camera position
            if(action == MouseAction::RELEASE )
            {
                dragging_position = false;
                auto mouse_pos = mouse.get_cursor_pos();
                drag_x = mouse_pos.x;
                drag_y = mouse_pos.y;
            }
        }
        //Handle Mouse rotate camera
        else if(button == MouseButton::RIGHT_MOUSE)
        {
            if(action == MouseAction::HOLD)
            {
                auto mouse_pos = mouse.get_cursor_pos();
                camera_rotating = true;
                rotating_x = mouse_pos.x;
                rotating_y = mouse_pos.y;
            }

            //Handle Mouse Drag Camera position
            if(action == MouseAction::RELEASE )
                camera_rotating = false;
        }
    };

    //set the mouse dragging_position callback
    std::function<void(int,int)> mouse_drag = [=](int x,int y)->void
    {
        //This will make the mouse drags the camera
        if(dragging_position)
        {
            //Calculate the offsets in comparison to previous drag_x and drag_y
            auto horizontal_delta = drag_x - x ;
            auto vertical_delta   = drag_y - y ;

            camera_component->position += camera_component->euler.right *(float)horizontal_delta * drag_speed;
            camera_component->position += camera_component->euler.up    * (float)vertical_delta  * drag_speed;
            //Canella::Logger::Info("Event input is working %d %d",x,y);
            drag_x = x;
            drag_y = y;
        }
        //Handles the left mouse camera rotation
        else if(camera_rotating)
        {
            //Calculate the offsets in comparison to previous drag_x and drag_y
            auto horizontal_delta = rotating_x - x ;
            auto vertical_delta = rotating_y - y ;
            //TODO investigate the camera orientation is messedup
            YAW   +=(float)horizontal_delta * sensitivity;
            PITCH += (float)vertical_delta * sensitivity;
            //Canella::Logger::Info("Event input is working %d %d",x,y);
            rotating_x = x;
            rotating_y = y;
        }
    };

    auto& mouse = Mouse::instance();
    Event_Handler<MouseButton,MouseAction> handler_click (mouse_press);
    Event_Handler<int,int> handler_move (mouse_drag);

    //Register the callbacks to mouse events
    mouse.OnMouseClick += handler_click;
    mouse.OnMouseMove  += handler_move;
}
