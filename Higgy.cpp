#include <iostream>
#include <json.hpp>
#include <fstream>
#include "Application/Application.h"
#include <glm/vec3.hpp>
int main()
{

	glm::vec3 point;
	//Load Config File (PASS THE RIGHT FOLDER)
	std::fstream f("../../../config/config.json");
	nlohmann::json j;
	f>>j;

	//Create Application
	Application::App Trotty;
	Trotty.initialize(j);


	return 0;
}
