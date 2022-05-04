#include <iostream>
#include <json.hpp>
#include <fstream>
#include "Application/Application.h"
int main()
{

	//Load Config File (PASS THE RIGHT FOLDER)
	std::fstream f("C://Users//icaro//OneDrive//Documentos//IcaroDev//Higgy//config//config.json");
	nlohmann::json j;
	f>>j;

	//Create Application
	Application::App Trotty;
	Trotty.initialize(j);
	Trotty.run();
	Trotty.close();

	return 0;
}
