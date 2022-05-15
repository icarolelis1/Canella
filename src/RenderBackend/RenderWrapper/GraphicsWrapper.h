#pragma once
#ifndef GRAPHICS_WRAPPER
#define GRAPHICS_WRAPPER
#endif

namespace RenderSystem{
    class GraphicsWrapper{
        public:
            virtual void initialize() = 0;
            virtual void render() = 0;
            virtual void destroy() = 0;
    };
}