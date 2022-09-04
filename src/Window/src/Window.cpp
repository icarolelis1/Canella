#include "Window/Window.h"
#include "Logger/Logger.hpp"
#include <vulkan/vulkan_win32.h>

void Canella::GlfwWindow::initialize(nlohmann::json &config)
{

	glfwInit();


	if (!glfwVulkanSupported()) {
		Canella::Logger::Error("Glfw doesnt support vulkan");
		return;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(config["Width"], config["Height"], config["Title"].get<std::string>().c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(m_window);
	glfwSetWindowUserPointer(m_window, this);

	// Initialize Inptus
	mouse = Mouse::getMouse();
	keyboard = KeyBoard::getKeyBoard();
	Mouse::getMouse().setWindowHandler(m_window);
	KeyBoard::getKeyBoard().setWindowHandler(m_window);

	//// Set Keyboard and mouse callbacks

	auto f = [](GLFWwindow *window, double xpos, double ypos)
	{
		static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window))->mouse.MouseCallback(window, xpos, ypos);
	};
	auto mouse_btn_callBack = [](GLFWwindow *window, int x, int y, int z)
	{
		static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window))->mouse.mouse_button_callback(window, x, y, z);
	};
	auto key_btn_callBack = [](GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window))->keyboard.key_callback(window, key, scancode, action, mods);
	};
	glfwSetCursorPosCallback(m_window, f);
	glfwSetMouseButtonCallback(m_window, mouse_btn_callBack);
	glfwSetKeyCallback(m_window, key_btn_callBack);
};
int Canella::GlfwWindow::shouldCloseWindow()
{
	return glfwWindowShouldClose(m_window);
}

Canella::GlfwWindow::~GlfwWindow()
{
	glfwDestroyWindow(m_window);
}

Extent &Canella::GlfwWindow::getExtent()
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	return Extent{static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
}

void Canella::GlfwWindow::update()
{
	glfwPollEvents();
}

void Canella::GlfwWindow::getSurface(VkInstance instance, VkSurfaceKHR *surface)
{
	auto r = glfwCreateWindowSurface(instance, m_window, nullptr, surface);

	if (r != VK_SUCCESS)
		Canella::Logger::Error("Failed to create window surface");

}

GLFWwindow *Canella::GlfwWindow::getHandle()
{
	return m_window;
}