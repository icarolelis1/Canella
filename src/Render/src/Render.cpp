#include "Render/Render.h"


Canella::Render::Render(Canella::RenderConfig& config) {
   
}


Canella::Render::Render(nlohmann::json &configFile)
{
    
};
Canella::RenderConfig & Canella::Render::getConfig() { return config; };

