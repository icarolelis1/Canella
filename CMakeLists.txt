# CMakeList.txt: projeto de CMake para Higgy, incluir origem e definir
# lógica específica a um projeto aqui.
#

cmake_minimum_required (VERSION 3.8)

project ("Higgy")
set(LIBS_DIR "./libs")
#Vulkan
find_package(Vulkan REQUIRED)


IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
SET(OS_LIBS_DIR ${LIBS_DIR}/linux)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF (WIN32)
# Linux specific code
SET(OS_LIBS_DIR ${LIBS_DIR}/win)
SET(OS_LIBS opengl32 Shlwapi)
ENDIF()
#GLFW
find_package(glfw3 REQUIRED PATHS  ${OS_LIBS_DIR}/glfw/lib/cmake/glfw3)

#GLM
find_package(glm REQUIRED PATHS  ${LIBS_DIR}/glm/lib/cmake/glm) 
# Adicione a origem ao executável deste projeto.

add_executable (Higgy Higgy.cpp)

include_directories(Higgy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlhoman 
                                    ${OS_LIBS_DIR}/glfw/include
                                    ${Vulkan_INCLUDE_DIRS} )

add_subdirectory(HiggySrc)

target_link_libraries(Higgy PRIVATE 
glfw

Vulkan::Vulkan
                        Render 
                        Engine
                        )


# TODO: Adicione testes e instale destinos, se necessário.
