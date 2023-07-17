#pragma once
#ifndef INPUTS
#define INPUTS
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "Eventsystem/Eventsystem.hpp"
namespace Canella
{
	enum InputAction;
	class KeyBoard
	{
	public:
		KeyBoard() = default;
		void setWindowHandler(GLFWwindow *window);
		bool getKeyPressed(int key);
		void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
		static KeyBoard &instance();
		Event<int, InputAction> OnKeyInput;

	private:
		bool key_pressing = false;
		bool keyboard_key_callback();
		GLFWwindow *_window;
	};

	typedef struct
	{
		double x;
		double y;
	} CursorPos;

	enum MouseButton
	{
		LEFT_MOUSE,
		RIGHT_MOUSE,
		MIDDLE_MOUSE
	};
	enum InputAction
	{
		PRESS,
		HOLD,
		RELEASE
	};

	class Mouse
	{
	public:
		Mouse() = default;
		Mouse(const Mouse &) = delete;
		Mouse operator=(const Mouse &) = delete;
		static Mouse &instance();
		void setWindowHandler(GLFWwindow *_window);
		bool get_mouse_action_status(int button, int action) const;
		GLFWmousebuttonfun mouse_button_callback(GLFWwindow *window, int x, int z, int y);
		void mouse_callback(GLFWwindow *window, double xpos, double ypos);

		CursorPos get_cursor_pos();
		GLFWwindow *window;
		Event<int, int> OnMouseMove;
		Event<MouseButton, InputAction> OnMouseClick;

	private:
		double x;
		double y;
		double lastX;
		double lastY;
		bool initialized = false;
		bool key_pressing = false;
	};
}

#endif