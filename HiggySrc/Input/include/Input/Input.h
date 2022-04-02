#pragma once
#ifndef INPUTS
#define INPUTS
#include <GLFW/glfw3.h>
namespace Engine{
    
    class KeyBoard{
        public:
            static KeyBoard* getKeyboard_Instance();
            void setWindowHandler(GLFWwindow *window);
            bool getKeyPressed(int key);
            void key_callback(GLFWwindow * window,int key, int scancode,int action, int mods);
            static KeyBoard* keyboard_instance;

        private:
             GLFWwindow *_window;
             KeyBoard();
    };
    
    typedef struct {
		double x;
		double y;
	}CursorPos;


    class Mouse{
    public:
        static Mouse* getMouseInstance();
		bool getMouseACtionStatus(int button, int action);
		void setWindowHandler(GLFWwindow* _window);
		CursorPos getCursorPos();        
		void MouseCallback(GLFWwindow* window, double xpos, double ypos);
         GLFWmousebuttonfun mouse_button_callback(GLFWwindow* window, int x, int z, int y);
		GLFWwindow* window;
        static Mouse* mouse_instace;
          private:
         Mouse();
         float x;
         float y;
         float lastX;
         float lastY;
         bool initialized = false;
    };
}

#endif