#include "Input/Input.h"
#include "Logger/Logger.hpp"

Canella::KeyBoard &Canella::KeyBoard::getKeyBoard()
{
	static KeyBoard keyboard;
	return keyboard;
}

void Canella::KeyBoard::setWindowHandler(GLFWwindow *_window)
{
	this->_window = _window;
};

bool Canella::KeyBoard::getKeyPressed(int key)
{
	if (glfwGetKey(this->_window, key))
	{
		return 1;
	}
	return 0;
};

void Canella::KeyBoard::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){};

void Canella::Mouse::setWindowHandler(GLFWwindow *windowHandler)
{
	this->window = windowHandler;
}

Canella::Mouse &Canella::Mouse::instance()
{
	static Mouse mouse;
	return mouse;
}

void Canella::Mouse::mouse_callback(GLFWwindow *window, double xpos, double ypos)
{

	if (!initialized)
	{
		lastX = xpos;
		lastY = ypos;
		initialized = true;
	}

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	x = xoffset;
	y = yoffset;
	lastX = xpos;
	lastY = ypos;
    OnMouseMove.invoke(lastX,lastY);
};
GLFWmousebuttonfun Canella::Mouse::mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {

    MouseButton canella_mouse_button;
    MouseAction canella_mouse_action;

    if (button == GLFW_MOUSE_BUTTON_LEFT)
        canella_mouse_button = MouseButton::LEFT_MOUSE;
    else if (button == GLFW_MOUSE_BUTTON_RIGHT)
        canella_mouse_button = MouseButton::RIGHT_MOUSE;
    else {
        canella_mouse_button = MouseButton::MIDDLE_MOUSE;
    }

    if (action == GLFW_PRESS && !key_pressing)
    {
        canella_mouse_action = MouseAction::PRESS;
        key_pressing = true;
    }
    if( action == GLFW_PRESS && key_pressing)
        canella_mouse_action = MouseAction::HOLD;

    if( action == GLFW_RELEASE )
    {
        canella_mouse_action = MouseAction::RELEASE;
       key_pressing = false;
    }
    OnMouseClick.invoke(canella_mouse_button,canella_mouse_action);
    return GLFWmousebuttonfun();
}

Canella::CursorPos Canella::Mouse::get_cursor_pos()
{
	return {lastX, lastY};
}

bool Canella::Mouse::get_mouse_action_status(int button, int action) const
{
	return (glfwGetMouseButton(window, button) == action);
}


