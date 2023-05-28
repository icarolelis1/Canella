#include <iostream>
#include <fstream>
#include <json.hpp>
#include "Project/Project.h"
#include "Logger/Logger.hpp"

Canella::Logger::Priority Canella::Logger::log_priority = Canella::Logger::Priority::Error_LOG;
std::mutex Canella::Logger::logger_mutex;



int main()
{
	std::fstream f("resources\\config\\config.json");
	nlohmann::json j;
	f >> j;

	// Create Application
	Canella::Project myApp;
	myApp.load(j);
	myApp.run();
	myApp.close();
 	return 0;
}
