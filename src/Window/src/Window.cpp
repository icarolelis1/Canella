#include "Window/Window.h"
#include "Logger/Logger.hpp"
#ifdef WIN32
#include <vulkan/vulkan_win32.h>
#include <Windows.h>
#endif

Canella::GlfwWindow* Canella::GlfwWindow::instance = nullptr;
void Canella::GlfwWindow::initialize(nlohmann::json &config) {
    glfwInit();
    if (!glfwVulkanSupported()) {
        Canella::Logger::Error("Glfw doesnt support vulkan");
        return;
    }
    title = config["Title"].get<std::string>();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    // glfwWindowHint(GLFW_DECORATED, false);
    auto width = config["Width"].get<std::uint32_t>();
    auto height = config["Height"].get<std::uint32_t>();

    m_window = glfwCreateWindow(width,
                                height,
                                title.c_str(),
                                nullptr, nullptr);

    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);
#ifdef WIN32
    RECT desktop;

    const HWND hDesktop = GetDesktopWindow();
    int horizontal, vertical;
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
    glfwSetWindowPos(m_window, horizontal / 2 - width / 2, vertical / 2 - height / 2);

#endif
    auto &mouse = Mouse::instance();
    auto &keyboard = KeyBoard::instance();
    mouse.setWindowHandler(m_window);
    keyboard.setWindowHandler(m_window);

    //// Set Keyboard and mouse callbacks
	auto mouse_pos_callback = [](GLFWwindow *window, double xpos, double ypos)
	{
        auto& mouse = Mouse::instance();
        mouse.mouse_callback(window,xpos,ypos);
	};
	auto mouse_btn_callBack = [](GLFWwindow *window, int x, int y, int z)
	{
        auto& mouse = Mouse::instance();
        mouse.mouse_button_callback(window, x, y, z);
	};
	auto key_btn_callBack = [](GLFWwindow *window, int key, int scancode, int action, int mods)
	{
        auto& keyboard = KeyBoard::instance();
        keyboard.key_callback(window, key, scancode, action, mods);
	};

    auto resize_callback = [](GLFWwindow *window, int width, int height)
    {
        if(window == nullptr) return;
        Extent ext(width,height);
        static_cast<GlfwWindow *>(glfwGetWindowUserPointer(window))->OnWindowResize.invoke(ext);
    };

	glfwSetCursorPosCallback(m_window, mouse_pos_callback);
	glfwSetMouseButtonCallback(m_window, mouse_btn_callBack);
	glfwSetKeyCallback(m_window, key_btn_callBack);
    glfwSetFramebufferSizeCallback(m_window,resize_callback);

};
int Canella::GlfwWindow::shouldCloseWindow()
{
	return glfwWindowShouldClose(m_window);
}

Canella::GlfwWindow::~GlfwWindow()
{
	glfwDestroyWindow(m_window);
    delete GlfwWindow::instance;
}

Extent Canella::GlfwWindow::getExtent()
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

    void Canella::GlfwWindow::set_title_data() {
    title += " ds";
    glfwSetWindowTitle(m_window,title.c_str());
}

void Canella::GlfwWindow::wait_idle() {
    int w, h = 0;
    glfwGetFramebufferSize(m_window, &w, &h);
    while (w == 0 || h == 0) {
        glfwGetFramebufferSize(m_window, &w, &h);
        glfwWaitEvents();
    }
    OnWindowFocus.invoke();
}

Canella::GlfwWindow &Canella::GlfwWindow::get_instance() {
        static GlfwWindow glfwWindow ;
        return glfwWindow;
}


