#pragma once
#ifndef COMPONENT_BASE
#define COMPONENT_BASE

class ComponentBase
{
    virtual void onStart() = 0;
    virtual void onUpdate(float dt) = 0;
};

#endif