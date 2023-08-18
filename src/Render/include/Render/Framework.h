#ifndef CANELLA_RENDER_FRAMEWORK
#define CANELLA_RENDER_FRAMEWORK
#include "Render.h"

namespace Canella
{
    //This is in progress slowly ...
    /**
     * @brief Creates a buffer with STORAGE bit set
     * @param render renderer
     * @param size size of the buffer
     * @param pdata initial data
     * @return unique id of the resource
     */
    uint64_t create_storage_buffer(Render* render,size_t size,void* pdata);

    /**
     * @brief Creates a device local Buffer to be used as uniforms
     * @param render render
     * @param size size of the buffer
     * @return unique id of the resource
     */
    uint64_t create_device_buffer(Render* render, size_t size);
}

#endif