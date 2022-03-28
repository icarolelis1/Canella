# CMakeList.txt: projeto de CMake para Higgy, incluir origem e definir
# lógica específica a um projeto aqui.
#
cmake_minimum_required (VERSION 3.8)

project ("Higgy")

#Vulkan
find_package(Vulkan REQUIRED )

# Adicione a origem ao executável deste projeto.

add_executable (Higgy Higgy.cpp )

include_directories(Higgy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlhoman)

add_subdirectory(HiggySrc)

target_include_directories(Higgy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/GLFW64/include)

target_link_libraries(Higgy PUBLIC Render Vulkan::Vulkan)
# TODO: Adicione testes e instale destinos, se necessário.
