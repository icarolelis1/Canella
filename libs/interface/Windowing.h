#pragma once
#ifndef WINDOWING
#define WINDOWING

struct Extent {
    unsigned int width, height;
};

class Windowing {
    virtual void update() = 0;
    virtual Extent getExtent() = 0;
};

#endif