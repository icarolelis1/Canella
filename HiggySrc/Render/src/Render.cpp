#include "Render/Render.h"

RenderSystem::Render::Render(){};
RenderSystem::RenderConfig&  RenderSystem::Render::getConfig(){return config;};
RenderSystem::Render RenderSystem::Render::createRender(nlohmann::json &configFile){return Render(); }