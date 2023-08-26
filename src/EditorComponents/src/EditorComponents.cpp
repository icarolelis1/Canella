#include "EditorComponents/EditorComponents.h"
#include "Window/Window.h"
#include "Project/Project.h"

using namespace Canella;

void CameraEditor::on_start()
{
    // Get windo width and height
    auto &window = Canella::GlfwWindow::get_instance();
    auto extent = window.getExtent();
    // Initialize camera_component projection matrix usign window width and height
    camera_component->projection = glm::perspectiveFov(glm::radians(70.0f),(float)extent.width,(float)extent.height,.01f, 1000.f);
    std::function<void(Extent)> resize_callback = [&](Extent extent)
    {
        if (extent.width == 0 || extent.height == 0)return;
        camera_component->projection = glm::perspectiveFov(glm::radians(70.0f),(float)extent.width,(float)extent.height,.01f, 1000.f);
    };
    // Create the event handler and register
    Event_Handler<Extent> resize_handler(resize_callback);
    window.OnWindowResize += resize_handler;
    // Todo try to improve this API too long
    set_mouse_callbacks();
}

/*
 * Updates the main camera editor
 */
void CameraEditor::on_update(float delta_time)
{
    time = delta_time;
    auto &mouse = Mouse::instance();
    auto pos = mouse.get_cursor_pos();
    auto &keyboard = KeyBoard::instance();
    last_x = pos.x;
    last_y = pos.y;
    update_euler_directions( delta_time );

    if (keyboard.getKeyPressed(GLFW_KEY_W))
        camera_component->entity_transform->position += camera_component->euler.front * speed * delta_time;
    if (keyboard.getKeyPressed(GLFW_KEY_S))
        camera_component->entity_transform->position -= camera_component->euler.front * speed * delta_time;
    if (keyboard.getKeyPressed(GLFW_KEY_A))
        camera_component->entity_transform->position -= camera_component->euler.right * speed * delta_time;
    if (keyboard.getKeyPressed(GLFW_KEY_D))
        camera_component->entity_transform->position += camera_component->euler.right * speed * delta_time;
    if (keyboard.getKeyPressed(GLFW_KEY_SPACE))
        camera_component->entity_transform->position += camera_component->euler.up * speed * delta_time;
    if (keyboard.getKeyPressed(GLFW_KEY_C))
        camera_component->entity_transform->position -= camera_component->euler.up * speed * delta_time;
}


void CameraEditor::update_euler_directions( float d ) {

    glm::mat4 rotate          = glm::mat4_cast( camera_component->entity_transform->orientation );
    auto      cam_pos_flipped = camera_component->entity_transform->position * glm::vec3( 1, 1, 1 );
    glm::mat4 translate       = glm::mat4( 1.0f );
    //translate = glm::translate( translate, -camera_component->entity_transform->position );

    translate = glm::translate( translate, -cam_pos_flipped );
    camera_component->view = rotate * translate;

    //camera_component->view = glm::inverse(camera_component->entity_transform->model_matrix);
    auto inv_view   = glm::inverse(camera_component->view);
    camera_component->euler.up    = glm::normalize( inv_view[1] );
    camera_component->euler.front = -glm::normalize( inv_view[2] );
    camera_component->euler.right = glm::normalize(glm::cross( camera_component->euler.up ,camera_component->euler.front));
}

void CameraEditor::set_mouse_callbacks()
{
    // Set the mouse click callback
    std::function<void(MouseButton, InputAction)> mouse_press = [=](MouseButton button, InputAction action) -> void
    {
        auto &mouse = Mouse::instance();
        auto pos = mouse.get_cursor_pos();
        drag_x = pos.x;
        drag_y = pos.y;

        if (button == MouseButton::MIDDLE_MOUSE)
        {
            if (action == InputAction::HOLD)
                dragging_position = true;

            // Handle Mouse Drag Camera position
            if (action == InputAction::RELEASE)
            {
                dragging_position = false;
                auto mouse_pos = mouse.get_cursor_pos();
                drag_x = mouse_pos.x;
                drag_y = mouse_pos.y;
            }
        }
            // Handle Mouse rotate camera
        else if (button == MouseButton::RIGHT_MOUSE)
        {
            if (action == InputAction::HOLD)
            {
                auto mouse_pos = mouse.get_cursor_pos();
                camera_rotating = true;
                rotating_x = mouse_pos.x;
                rotating_y = mouse_pos.y;
            }

            // Handle Mouse Drag Camera position
            if (action == InputAction::RELEASE)
                camera_rotating = false;
        }
    };

    // set the mouse dragging_position callback
    std::function<void(int, int)> mouse_drag = [=](int x, int y) -> void
    {
        // This will make the mouse drags the camera
        if (dragging_position)
        {
            // Calculate the offsets in comparison to previous drag_x and drag_y
            auto horizontal_delta = drag_x - x;
            auto vertical_delta = drag_y - y;

            camera_component->entity_transform->position += camera_component->euler.right * (float)horizontal_delta * drag_speed;
            camera_component->entity_transform->position += camera_component->euler.up * (float)vertical_delta * drag_speed;
            // Canella::Logger::Info("Event input is working %d %d",x,y);
            drag_x = x;
            drag_y = y;
        }
            // Handles the left mouse camera rotation
        else if (camera_rotating)
        {
            // Calculate the offsets in comparison to previous drag_x and drag_y
            auto horizontal_delta = rotating_x - x;
            auto vertical_delta = rotating_y - y;
            camera_component->yaw += horizontal_delta  * sensitivity;
            camera_component->pitch += vertical_delta  * sensitivity;
            auto quat_yaw = glm::normalize(glm::angleAxis( (float)camera_component->yaw,glm::vec3(0,1,0)));
            auto quat_pitch = glm::normalize(glm::angleAxis( camera_component->pitch ,glm::vec3(1,0,0)));

            camera_component->entity_transform->orientation = (quat_pitch * quat_yaw );
            // Canella::Logger::Info("Event input is working %d %d",x,y);
            rotating_x = x;
            rotating_y = y;
        }
    };

    auto &mouse = Mouse::instance();
    Event_Handler<MouseButton, InputAction> handler_click(mouse_press);
    Event_Handler<int, int> handler_move(mouse_drag);

    // Register the callbacks to mouse events
    mouse.OnMouseClick += handler_click;
    mouse.OnMouseMove += handler_move;
}

void CameraEditor::move_and_focus( glm::vec3 position ) {
    auto direction = glm::normalize(position - camera_component->entity_transform->position);
    auto quat = glm::quatLookAt(direction,glm::vec3(0,1,0));
    camera_component->entity_transform->orientation = quat;
}





























