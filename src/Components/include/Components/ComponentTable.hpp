//
// Created by icaro on 30/08/2023.
//
#ifndef COMPONENT_TABLE
#define COMPONENT_TABLE
#define stringify( name ) #name
#include "unordered_map"
namespace Canella
{
    enum ComponentType
    {
        Transform
    };

    static std::unordered_map<const char*,ComponentType> const table = {
            {stringify(ComponentType::Transform),ComponentType::Transform}
    };

}
#endif