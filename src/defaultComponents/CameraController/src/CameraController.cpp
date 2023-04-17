#include "CameraController/CameraController.h"
#include "Window/Window.h"
/**
 * \brief Camera Controller Component
 * \param _id 
 */
Canella::CameraController::CameraController(std::string _id) : Component(_id), ROLL(0), euler(), position()
{
    Logger::Info("Created CameraController");
}

std::shared_ptr<Canella::Component> Canella::CameraController::create()
{
    std::shared_ptr<Component> m = std::make_shared<CameraController>("1");
    return m;
};

void Canella::CameraController::onStart()
{
    position = glm::vec3(0, .1, -3);
    updateDirections();
}

void Canella::CameraController::loadState(nlohmann::json &config)
{
}

void Canella::CameraController::saveState(nlohmann::json &config)
{
}

void Canella::CameraController::updateDirections()
{
    glm::vec3 front;
    front.x = cos(glm::radians(YAW)) * cos(glm::radians(PITCH));
    front.y = sin(glm::radians(PITCH));
    front.z = sin(glm::radians(YAW)) * cos(glm::radians(PITCH));
    euler.front = glm::normalize(front);

    glm::vec3 WorldUp = glm::vec3(0, -1, 0);
    euler.right = glm::normalize(glm::cross(euler.front, WorldUp));
    euler.up = glm::normalize(glm::cross(euler.right, euler.front));
}

void Canella::CameraController::onUpdate(float timeStep)
{

    if (KeyBoard::getKeyBoard().getKeyPressed(GLFW_KEY_W))
        position += (euler.front * timeStep * velocity);

    if (KeyBoard::getKeyBoard().getKeyPressed(GLFW_KEY_S))
        position += (-euler.front * timeStep * velocity);

    if (KeyBoard::getKeyBoard().getKeyPressed(GLFW_KEY_A))
        YAW += (timeStep * velocity);

    if (KeyBoard::getKeyBoard().getKeyPressed(GLFW_KEY_D))
        YAW -= (timeStep * velocity);

    if (KeyBoard::getKeyBoard().getKeyPressed(GLFW_KEY_R))
        PITCH += (timeStep * velocity);

    if (KeyBoard::getKeyBoard().getKeyPressed(GLFW_KEY_F))
        PITCH -= (timeStep * velocity);

    Logger::Info("Updating");
    // if (Window::keyboard.getKeyPressed(GLFW_KEY_SPACE))
    // {
    //     transform->increasePos(-camera->eulerDirections.up * timeStep * velocity / 5.0f);
    // }

    // else if (Window::keyboard.getKeyPressed(GLFW_KEY_C))
    // {
    //     transform->increasePos(camera->eulerDirections.up * timeStep * velocity / 5.0f);
    // }

    // if (Window::keyboard.getKeyPressed(GLFW_KEY_1))
    // {
    //     resetCamera();
    // }

    // auto offset = 40;
    // Engine::CursorPos cursorPos = Window::mouse.getCursorPos();

    // if (Window::mouse.getMouseACtionStatus(GLFW_MOUSE_BUTTON_2, GLFW_PRESS))
    // {
    //     if (cursorPos.x >= 1920 - offset)
    //     {
    //         transform->increasePos(velocity * camera->eulerDirections.right);
    //     }
    //     if (cursorPos.x <= offset)
    //         transform->increasePos(-velocity * camera->eulerDirections.right);

    //     if (cursorPos.y >= 1055 - offset)
    //         transform->increasePos(-velocity * glm::normalize(glm::vec3(camera->eulerDirections.front.x, 0, camera->eulerDirections.front.z) * glm::cos(glm::radians(-90 - PITCH))));
    //     if (cursorPos.y <= (offset))
    //         transform->increasePos(velocity * glm::normalize(glm::vec3(camera->eulerDirections.front.x, 0, camera->eulerDirections.front.z) * glm::cos(glm::radians(-90 - PITCH))));
    // }

    updateDirections();
}
