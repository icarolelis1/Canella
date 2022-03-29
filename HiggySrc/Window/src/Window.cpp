#include "Window/Window.h"


Engine::Window::Window(){};
void Engine::Window::initialize(nlohmann::json& config){
     glfwInit();
     GLFWwindow *w = glfwCreateWindow(600,600,"TESTING",nullptr,nullptr);

     while(!glfwWindowShouldClose(w)){
         glfwSwapBuffers(w);
         glfwPollEvents();
     }   
}