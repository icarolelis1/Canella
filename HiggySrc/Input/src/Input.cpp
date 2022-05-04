#include "Input/Input.h"
Engine::Mouse* Engine::Mouse::mouse_instace = nullptr;
Engine::KeyBoard* Engine::KeyBoard::keyboard_instance = nullptr;

Engine::KeyBoard::KeyBoard(){};
Engine::Mouse::Mouse(){};

void Engine::KeyBoard::setWindowHandler(GLFWwindow* _window){
    keyboard_instance->_window = _window;
};

bool  Engine::KeyBoard::getKeyPressed(int key){
    if (glfwGetKey(this->_window, key)) {
		return 1;
	}		
	return 0;
};

Engine::KeyBoard* Engine::KeyBoard::getKeyboard_Instance(){
    if(Engine::KeyBoard::keyboard_instance == nullptr)
        Engine::KeyBoard::keyboard_instance = new KeyBoard();
    return Engine::KeyBoard::keyboard_instance;
}

void Engine::KeyBoard::key_callback(GLFWwindow * window,int key, int scancode,int action, int mods){};

void Engine::Mouse::setWindowHandler(GLFWwindow* windowHandler){
    Engine::Mouse::mouse_instace->window = windowHandler;
}

 Engine::Mouse* Engine::Mouse::getMouseInstance(){
     
     if(Engine::Mouse::mouse_instace == nullptr)
        Engine::Mouse::mouse_instace = new Engine::Mouse();

        return mouse_instace;
 };
 void Engine::Mouse::MouseCallback(GLFWwindow * window, double xpos,double ypos){

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
 GLFWmousebuttonfun Engine::Mouse::mouse_button_callback(GLFWwindow* window, int x, int z, int y){
    return GLFWmousebuttonfun();
 }
 Engine::CursorPos Engine::Mouse::getCursorPos(){
     CursorPos p = { lastX,lastY };
	return p;
 }

bool Engine::Mouse::getMouseACtionStatus(int button ,   int action){
    	return (glfwGetMouseButton(window, button) == action);
}