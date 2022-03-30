#include  "Application/Application.h"

Application::App::App(){};       
void Application::App::initialize(nlohmann::json& config){
    
    appName = config["App"]["Name"];
    AppVersion =  config["App"]["AppVersion"];
    graphicsAPI =  config["App"]["Api"];
    std::cout<<"\nBuilding App: " <<config["App"]["Name"]<<std::endl;
    std::cout<<"  AppVersion : " <<config["App"]["AppVersion"]<<std::endl;


}   