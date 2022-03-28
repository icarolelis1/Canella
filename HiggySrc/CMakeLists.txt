cmake_minimum_required (VERSION 3.8)

add_library(Render Render/src/Render.cpp

)

target_include_directories(Render PUBLIC Render/include)

