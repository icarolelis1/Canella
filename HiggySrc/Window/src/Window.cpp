#include "Window/Window.h"


Engine::Window::Window(){};
void Engine::Window::initialize(nlohmann::json& config){
     glfwInit();
     int width = config["Width"];
     m_window = glfwCreateWindow(config["Width"],config["Height"],config["Title"].get<std::string>().c_str(),nullptr,nullptr);
    glfwMakeContextCurrent(m_window);   
    glfwSetWindowUserPointer(m_window, this);
    glfwInitHint(GLFW_NO_API,0);

    //Initialize Inptus
    mouse =  Mouse::getMouseInstance();
    keyboard = KeyBoard::getKeyboard_Instance();
    Mouse::mouse_instace->setWindowHandler(m_window);
    KeyBoard::keyboard_instance->setWindowHandler(m_window);

    //Set Keyboard and mouse callbacks

	auto f = [](GLFWwindow* window, double xpos, double ypos) {
		static_cast<Window*>(glfwGetWindowUserPointer(window))->mouse->MouseCallback(window,xpos,ypos);
	};
	auto mouse_btn_callBack = [](GLFWwindow* window, int x,int y,int z) {
		static_cast<Window*>(glfwGetWindowUserPointer(window))->mouse->mouse_button_callback(window,x,y,z);
	};
    auto key_btn_callBack = [](GLFWwindow * window,int key, int scancode,int action, int mods) {
		static_cast<Window*>(glfwGetWindowUserPointer(window))->keyboard->key_callback(window,key,scancode,action,mods);
	};
	glfwSetCursorPosCallback(m_window, f);
	glfwSetMouseButtonCallback(m_window,mouse_btn_callBack);
    glfwSetKeyCallback(m_window,key_btn_callBack);

    while(!glfwWindowShouldClose(m_window)){

        glfwPollEvents();
        if(keyboard->getKeyPressed(GLFW_KEY_A)){
            std::cout<<"A";
        }

        Engine::CursorPos p = mouse->getCursorPos();
        std::cout<<"X "<<p.x <<" Y "<<p.y <<std::endl;

    }

};