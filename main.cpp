#include <iostream>
#include <fstream>
#include <json.hpp>
#include "Application/Application.h"
#include "Logger/Logger.hpp"

Canella::Logger::Priority Canella::Logger::log_priority = Canella::Logger::Priority::Error_LOG;
std::mutex Canella::Logger::logger_mutex;
#define GLFW_INCLUDE_VULKAN
#define  GLFW_EXPOSE_NATIVE_WIN32
int main()
{
	Canella::Logger::Info("INFO");
	Canella::Logger::Debug("DEBUG");
	Canella::Logger::Error("ERROR");
	Canella::Logger::Warn("WARN");
	Canella::Logger::Trace("TRACE");

	std::fstream f("resources\\config\\config.json");
	nlohmann::json j;
	f >> j;

	// Create Application
	Application::App myApp;
	myApp.initialize(j);
	myApp.run();
	myApp.close();
	return 0;
}
