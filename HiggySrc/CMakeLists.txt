cmake_minimum_required (VERSION 3.8)

add_library(Render Render/src/Render.cpp
            Device/src/Device.cpp 
            Stb/stb.h  


)

target_include_directories(Render PUBLIC Render/include
                            Image/include
                            Device/include
                            Stb/
                            ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/stb 

)


add_library(Engine Window/src/Window.cpp
                            Component/src/Component.cpp
                            Entity/src/Entity.cpp  
                            Input/src/Input.cpp
                            Scenegraph/src/Scenegraph.cpp
                            defaultComponents/Transform/src/Transform.cpp
                            ComponentRegistry/src/ComponentRegistry.cpp

)


target_include_directories(Engine PUBLIC Window/include
                                        Component/include   
                                         Entity/include
                                         Input/include
                                         Scenegraph/include
                                         defaultComponents/Transform/include
                                         ComponentRegistry/include


)

add_library(Application  Application/src/Application.cpp        
                            
)

target_link_libraries(Application PUBLIC Engine Render  )

target_include_directories(Application PUBLIC Application/include
                                            
)