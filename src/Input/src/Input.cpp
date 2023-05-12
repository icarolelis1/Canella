#include "Input/Input.h"

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

Canella::Mouse &Canella::Mouse::getMouse()
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
		initialized = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	x = xoffset;
	y = yoffset;
	lastX = xpos;
	lastY = ypos;
};
GLFWmousebuttonfun Canella::Mouse::mouse_button_callback(GLFWwindow *window, int x, int z, int y)
{
	return GLFWmousebuttonfun();
}

Canella::CursorPos Canella::Mouse::getCursorPos()
{
	CursorPos p = {lastX, lastY};
	return p;
}

bool Canella::Mouse::get_mouse_action_status(int button, int action) const
{
	return (glfwGetMouseButton(window, button) == action);
}