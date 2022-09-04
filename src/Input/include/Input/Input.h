#pragma once
#ifndef INPUTS
#define INPUTS
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
namespace Canella {

	class KeyBoard {
	public:
		KeyBoard() = default;
		void setWindowHandler(GLFWwindow* window);
		bool getKeyPressed(int key);
		void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static KeyBoard& getKeyBoard();

	private:
		GLFWwindow* _window;
	};

	typedef struct {
		double x;
		double y;
	}CursorPos;


	class Mouse {
	public:
		Mouse() = default;
		static Mouse& getMouse();
		bool getMouseACtionStatus(int button, int action);
		void setWindowHandler(GLFWwindow* _window);
		CursorPos getCursorPos();
		void MouseCallback(GLFWwindow* window, double xpos, double ypos);
		GLFWmousebuttonfun mouse_button_callback(GLFWwindow* window, int x, int z, int y);
		GLFWwindow* window;

	private:
		float x;
		float y;
		float lastX;
		float lastY;
		bool initialized = false;
	};
}

#endif