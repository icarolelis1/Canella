#pragma once
#ifndef RENDER
#define RENDER
#include "Windowing.h"
#include "RenderBase.h"
#include <json.hpp>
#include <iostream>
#include <unordered_map>

namespace Canella{
    
    class Render : public RenderBase{
        public:
            Render() = default;
            Render(nlohmann::json& json);
            virtual void render() = 0;
            virtual void update(float time) = 0;
    };
}
#endif