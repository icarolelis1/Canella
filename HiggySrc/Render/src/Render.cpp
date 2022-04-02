#include "Render/Render.h"

 RenderSystem::Render* RenderSystem::Render::render_instance = nullptr;
RenderSystem::Render::Render(){};
RenderSystem::RenderConfig&  RenderSystem::Render::getConfig(){return config;};
RenderSystem::Render* RenderSystem::Render::createRender(nlohmann::json &configFile){

    if(render_instance == nullptr){
        render_instance = new RenderSystem::Render();
    };
            return  render_instance;
};