#pragma once
#ifndef WINDOWING
#define WINDOWING
#include "Eventsystem/Eventsystem.hpp"

struct Extent {
    Extent()  = default;
    Extent(unsigned int w,unsigned int h)
    {
        width = w;
        height = h;
    }
    unsigned int width, height;
};

class Windowing {
    virtual void update() = 0;
    virtual Extent getExtent() = 0;
    virtual void set_title_data() = 0;

};

#endif