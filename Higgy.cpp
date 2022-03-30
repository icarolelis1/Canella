#include <iostream>
#include <json.hpp>
#include <fstream>
#include "Application/Application.h"

int main()
{

	//Load Config File (PASS THE RIGHT FOLDER)
	std::fstream f("../../../config.json");
	nlohmann::json j;
	f>>j;

	//Create Application
	Application::App Trotty;
	Trotty.initialize(j);


	return 0;
}
