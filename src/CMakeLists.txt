cmake_minimum_required (VERSION 3.8)

add_library(
            Render 
            RenderBackend/VulkanBackend/Instance/src/Instance.cpp
            RenderBackend/VulkanBackend/Device/src/Device.cpp
            RenderBackend/VulkanBackend/Swapchain/src/Swapchain.cpp
            RenderBackend/VulkanBackend/Image/src/Image.cpp
            RenderBackend/VulkanBackend/VulkanRender/src/VulkanRender.cpp

            Render/src/Render.cpp
            Stb/stb.h  
)

target_include_directories(
                            Render PUBLIC 
                            Stb/
                            RenderBackend/VulkanBackend/Instance/include
                            RenderBackend/VulkanBackend/Device/include
                            RenderBackend/VulkanBackend/Swapchain/include
                            RenderBackend/VulkanBackend/Image/include
                            RenderBackend/VulkanBackend/Swapchain/include
                            RenderBackend/VulkanBackend/VulkanRender/include
                            RenderBackend/RenderWrapper

                            Render/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/stb 


)


add_library(Engine Window/src/Window.cpp
                            Component/src/Component.cpp
                            Entity/src/Entity.cpp  
                            Input/src/Input.cpp
                            Scenegraph/src/Scenegraph.cpp
                            defaultComponents/Transform/src/Transform.cpp
                            ComponentRegistry/src/ComponentRegistry.cpp
                            Eventsystem/src/Eventsystem.cpp

)


target_include_directories(Engine PUBLIC Window/include
                                        Component/include   
                                         Entity/include
                                         Input/include
                                         Scenegraph/include
                                         defaultComponents/Transform/include
                                         ComponentRegistry/include
                                         Threadpool/include
                                         Eventsystem/include
                                         Logger/include


)

add_library(Application  Application/src/Application.cpp        
                            
)

target_link_libraries(Application PUBLIC Engine Render  )

target_include_directories(Application PUBLIC Application/include
                                            
)