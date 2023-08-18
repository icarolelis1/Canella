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
    camera_component->projection = glm::perspectiveFov(glm::radians(70.0f),
                                                       (float)extent.width,(float)extent.height,
                                                       .01f, 1000.f);
    //camera_component->projection[1][1] *=-1.0f;
    // Attach Window Resize Event Callback.
    // Reconstruct the projection matrix when the window resizes
    std::function<void(Extent)> resize_callback = [&](Extent extent)
    {
        if (extent.width == 0 || extent.height == 0)
            return;
        camera_component->projection = glm::perspectiveFov(glm::radians(70.0f),
                                                           (float)extent.width,(float)extent.height,
                                                           .01f, 1000.f);
        camera_component->projection[1][1] *=-1.0f;

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
    auto &camera_position = camera_component->entity_transform->position;
    auto &camera_euler = camera_component->euler;
    auto &mouse = Mouse::instance();
    auto pos = mouse.get_cursor_pos();
    auto &keyboard = KeyBoard::instance();
    last_x = pos.x;
    last_y = pos.y;
/*    if (camera_component->entity_transform->rotation.x > 89.0f)
        camera_component->entity_transform->rotation.x = 89.0f;
    if (camera_component->entity_transform->rotation.x < -89.0f)
        camera_component->entity_transform->rotation.x = -89.0f;*/

    camera_input_keys();
    update_euler_directions();

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

    //Logger::Debug("%f", camera_component->entity_transform->position.y);
}

void CameraEditor::camera_input_keys()
{
    auto key_callbacks = [=](int key, InputAction action)
    {
        if (key == GLFW_KEY_W && action == InputAction::HOLD)
            camera_component->entity_transform->position += camera_component->euler.front * speed * time;
        if (key == GLFW_KEY_S && action == InputAction::HOLD)
            camera_component->entity_transform->position -= camera_component->euler.front * speed * time;
        if (key == GLFW_KEY_A && action == InputAction::HOLD)
            camera_component->entity_transform->position -= camera_component->euler.right * speed * time;
        if (key == GLFW_KEY_D && action == InputAction::HOLD)
            camera_component->entity_transform->position += camera_component->euler.right * speed * time;
        if (key == GLFW_KEY_SPACE && action == InputAction::HOLD)
            camera_component->entity_transform->position += camera_component->euler.up * speed * time;
        if (key == GLFW_KEY_C && action == InputAction::HOLD)
            camera_component->entity_transform->position -= camera_component->euler.up * speed * time;
    };

    // KeyBoard::instance().OnKeyInput += Event_Handler<int, InputAction>(key_callbacks);
}

void CameraEditor::update_euler_directions()
{
    glm::quat qPitch = glm::angleAxis(glm::radians(camera_component->entity_transform->rotation.x), glm::vec3(1, 0, 0));
    glm::quat qYaw   = glm::angleAxis(glm::radians(camera_component->entity_transform->rotation.y), glm::vec3(0, 1, 0));
    glm::quat qRoll  = glm::angleAxis(glm::radians(camera_component->entity_transform->rotation.z), glm::vec3(0, 0, 1));

    // For a FPS camera we can omit roll
    orientation = qPitch * qYaw * qRoll;
    orientation = glm::normalize(orientation);
    glm::mat4 rotate = glm::mat4_cast(orientation);
    auto cam_pos_flipped = camera_component->entity_transform->position * glm::vec3(1,1,1);
    glm::mat4 translate = glm::mat4(1.0f);
    translate = glm::translate(translate,-camera_component->entity_transform->position);

    translate = glm::translate(translate, -cam_pos_flipped);
    camera_component->view = rotate * translate;

    //camera_component->view = glm::inverse(camera_component->entity_transform->model_matrix);

    camera_component->euler.up = glm::normalize(glm::vec3(0.f,  1.f, 0.0f) * orientation);
    camera_component->euler.front = glm::normalize(glm::vec3(0.f, 0.f, -1.0f) * orientation);
    camera_component->euler.right = glm::normalize(glm::vec3(1.f, 0.f, 0.0f) * orientation);
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
            // TODO investigate the camera orientation is messedup
            camera_component->entity_transform->rotation.y -= (float)horizontal_delta * sensitivity;
            camera_component->entity_transform->rotation.x += (float)vertical_delta * sensitivity;
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
