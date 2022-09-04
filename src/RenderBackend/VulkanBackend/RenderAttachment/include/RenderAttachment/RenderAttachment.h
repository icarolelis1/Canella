#pragma once
#ifndef RENDER_ATTACHMENT
#define RENDER_ATTACHMENT
#include <vector>
namespace Canella{
namespace RenderSystem{
namespace VulkanBackend{

class Image;

class RenderAttachment{

    RenderAttachment(std::vector<Image>&& images);
    


};




}
}
}


#endif