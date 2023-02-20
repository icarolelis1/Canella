cmake_minimum_required (VERSION 3.8)
set (CMAKE_CXX_STANDARD 17)

project ("Canella")
set(LIBS_DIR "./libs")

# VULKAN AND GLFW FOLDER
IF(WIN32)
SET(GLFW_PATH C:/glfw)
endif()

add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)

find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
message(STATUS "Found Package Vulkan: $ENV{VULKAN_SDK}")

if (NOT VULKAN_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
else()
	message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
SET(OS_LIBS_DIR ${LIBS_DIR}/linux)
add_definitions( -DLINUX=1 )
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


IF (WIN32)
add_definitions( -DWIN32=1 )
# Linux specific code
SET(OS_LIBS_DIR ${LIBS_DIR}/win)
ENDIF()

#GLFW
if (DEFINED GLFW_PATH)
  message(STATUS "Using GLFW path specified in .env")
  set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")
  if (MSVC)
    set(GLFW_LIB "${GLFW_PATH}/lib-vc2022") #
  elseif (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    message(STATUS "USING MINGW")
    set(GLFW_LIB "${GLFW_PATH}/lib-mingw-w64") # 
  endif()
else()
  find_package(glfw3 3.3 REQUIRED)
  set(GLFW_LIB glfw)
  message(STATUS "Found GLFW")
endif()
if (NOT GLFW_LIB)
	message(FATAL_ERROR "Could not find glfw library!")
else()
	message(STATUS "Using glfw lib at: ${GLFW_LIB}")
endif()


#GLM
find_package(glm REQUIRED PATHS  ${LIBS_DIR}/glm/lib/cmake/glm) 

add_executable (${PROJECT_NAME} main.cpp)

target_link_directories(${PROJECT_NAME} PUBLIC $ENV{VULKAN_SDK}/Lib ${GLFW_LIB} )

include_directories(${PROJECT_NAME} PUBLIC 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/nlhoman 
                                    ${GLFW_INCLUDE_DIRS}
                                    ${LIBS_DIR}/glm/include
                                    $ENV{VULKAN_SDK}/Include
                                    ${LIBS_DIR}/interface
)

add_subdirectory(src)

IF(WIN32)

            target_link_libraries(${PROJECT_NAME} PRIVATE 
            glfw3
            Application
            vulkan-1  )

ELSEIF(UNIX)
            target_link_libraries(${PROJECT_NAME} PRIVATE 
            ${GLFW_LIB}
            Application
            vulkan-1 )

endif()

