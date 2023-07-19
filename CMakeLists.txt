cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

project("Canella")
set(LIBS_DIR "./libs")

# VULKAN AND GLFW FOLDER

add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)

find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
message(STATUS "Found Package Vulkan: $ENV{VULKAN_SDK}")

if(NOT VULKAN_FOUND)
  message(FATAL_ERROR "Could not find Vulkan library!")
else()
  message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif()

find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)


IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(OS_LIBS_DIR ${LIBS_DIR}/linux)
  add_definitions(-DLINUX=1)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(WIN32)
  add_definitions(-DWIN32=1)

  # Linux specific code
  SET(OS_LIBS_DIR ${LIBS_DIR}/win)
ENDIF()

SET(GLFW_PATH ${OS_LIBS_DIR}/glfw)
# GLFW
if(DEFINED GLFW_PATH)
  message(STATUS "Using GLFW path specified in .env")
  set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")

  if(MSVC)
    message(STATUS "USING MSVC")
    set(GLFW_LIB "${GLFW_PATH}/lib/lib-vc2022") #
  elseif(CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    message(STATUS "USING MINGW")
    set(GLFW_LIB "${GLFW_PATH}/lib/lib-mingw-w64") #
  endif()
else()
  find_package(glfw3 3.3 REQUIRED)
  set(GLFW_LIB glfw)
  message(STATUS "Found GLFW")
endif()

if(NOT GLFW_LIB)
  message(FATAL_ERROR "Could not find glfw library!")
else()
  message(STATUS "Using glfw lib at: ${GLFW_LIB}")
endif()

#find_package(glfw3 3.3 REQUIRED PATHS ${OS_LIBS_DIR}/glfw/lib/cmake/glfw3)

# GLM
#find_package(glm REQUIRED PATHS ${LIBS_DIR}/glm/lib/cmake/glm)


add_executable(${PROJECT_NAME}  main.cpp src/Editor/src/Editor.cpp src/EditorComponents/src/EditorComponents.cpp)

target_link_directories(${PROJECT_NAME} PUBLIC $ENV{VULKAN_SDK}/Lib ${GLFW_LIB}  ${OS_LIBS_DIR}/assimp/lib/ ${OS_LIBS_DIR}/metis/lib/)


#Setup Imgui
file (GLOB IMGUI_SRC
        "src/ImGUI/*.cpp")
add_library(
        ImGUI
        ${IMGUI_SRC})

include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/nlhoman
  ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/entt
  ${GLFW_INCLUDE_DIRS}
  ${LIBS_DIR}/glm/include
  $ENV{VULKAN_SDK}/Include
  ${LIBS_DIR}/interface
  ${OS_LIBS_DIR}/assimp/include
  ${OS_LIBS_DIR}/metis/include
  src/Eventsystem/include
  src/Editor/include
  src/Logger/include
  src/EditorComponents/include
  src/ImGUI
        )

add_subdirectory(src)

IF(WIN32)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw3
    Engine
    vulkan-1
    assimp-vc143-mtd
    ImGUI
    metis
    )

ELSEIF(UNIX)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    ${GLFW_LIB}
    Engine
    vulkan-1)
endif()

set( ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders)
file(GLOB SHADERS ${SHADER_DIR}/*.vert ${SHADER_DIR}/*.frag ${SHADER_DIR}/*.comp ${SHADER_DIR}/*.geom ${SHADER_DIR}/*.tesc ${SHADER_DIR}/*.tese ${SHADER_DIR}/*.mesh ${SHADER_DIR}/*.task ${SHADER_DIR}/*.rgen ${SHADER_DIR}/*.rchit ${SHADER_DIR}/*.rmiss)

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME)
    add_custom_command(OUTPUT ${SHADER_DIR}/shaders/${FILENAME}.spv
        COMMAND ${glslc_executable} ${SHADER} -o ${SHADER_DIR}/spirv/${FILENAME}.spv  --target-spv=spv1.4
        DEPENDS ${SHADER}
        COMMENT "Compiling ${FILENAME}")
list(APPEND SPV_SHADERS ${SHADER_DIR}/shaders/${FILENAME}.spv)
endForeach()
add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})

add_custom_command(TARGET ${PROJECT_NAME}  POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/resources
    ${PROJECT_BINARY_DIR}/resources)

add_custom_command(TARGET ${PROJECT_NAME}  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/libs/dlls/assimp-vc143-mtd.dll"
        ${PROJECT_BINARY_DIR})

add_custom_target(copy-files ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/resources
        ${CMAKE_BINARY_DIR}/resources
        )