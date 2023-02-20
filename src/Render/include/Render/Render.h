#pragma once
#ifndef RENDER
#define RENDER
#include <json.hpp>
#include <iostream>
#include <stb.h>
#include "Windowing.h"
#include <unordered_map>

namespace Canella{
    
  
    class Render{
        public:
            Render() = default;
            Render(nlohmann::json& json);

            virtual void initialize(Windowing* window) = 0;

            virtual void render() = 0;

            virtual void update(float time) = 0;



    };
}
#endif