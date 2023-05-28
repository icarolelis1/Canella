#pragma once
#ifndef RENDER_BASE
#define RENDER_BASE

class RenderBase
{
    virtual void render() = 0;
    virtual void update(float time) = 0;
};

#endif