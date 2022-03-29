# CMakeList.txt: projeto de CMake para Higgy, incluir origem e definir
# lógica específica a um projeto aqui.
#

cmake_minimum_required (VERSION 3.8)

project ("Higgy")

#Vulkan
find_package(Vulkan REQUIRED)
#GLFW
link_directories(${CMAKE_SOURCE_DIR}/third_party/GLFW64/lib-vc2015)

# Adicione a origem ao executável deste projeto.

add_executable (Higgy Higgy.cpp)

include_directories(Higgy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlhoman ${CMAKE_CURRENT_SOURCE_DIR}/third_party/GLFW64/include ${Vulkan_INCLUDE_DIRS} )

add_subdirectory(HiggySrc)

target_link_libraries(Higgy PRIVATE 
glfw3

Vulkan::Vulkan
                        Render 
                        Engine
                        )


# TODO: Adicione testes e instale destinos, se necessário.
