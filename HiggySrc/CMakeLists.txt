cmake_minimum_required (VERSION 3.8)

add_library(Render Render/src/Render.cpp
            Device/src/Device.cpp

)

target_include_directories(Render PUBLIC Render/include
                            Image/include
                            Device/include

)


add_library(Engine Window/src/Window.cpp)


target_include_directories(Engine PUBLIC Window/include

)
