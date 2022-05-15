#include <iostream>
#include <json.hpp>
#include <fstream>
#include "Application/Application.h"
#include "Eventsystem/Eventsystem.h"
#include "Logger/Logger.hpp"
Engine::Logger::Priority Engine::Logger::log_priority = Engine::Logger::Priority::Critial_LOG;
std::mutex Engine::Logger::logger_mutex;
#define WIN32
int main()
{

	// Engine::Event_Handler<int, char *> ev([](int i, char *k)
	// 									  { Engine::Logger::Info("Show %d %s", i, k); });
	// Engine::Event<int, char *> myevent;
	// myevent += (ev);
	// myevent-=  (ev);
	// myevent.call(1, "2");
	// myevent.call(1, "2");
	// myevent.call(1, "2");

	// Engine::Logger::Info("INFO");
	// Engine::Logger::Debug("DEBUG");
	// Engine::Logger::Error("ERROR");
	// Engine::Logger::Warn("WARN");
	// Engine::Logger::Trace("TRACE");

	// Load Config File (PASS THE RIGHT FOLDER)
	std::fstream f("C://Users//icaro//OneDrive//Documentos//IcaroDev//Canella//config//config.json");
	nlohmann::json j;
	f >> j;

	// Create Application
    Application::App Trotty;
    Trotty.initialize(j);
    Trotty.run();
    Trotty.close();

	return 0;
}
