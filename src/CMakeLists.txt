cmake_minimum_required(VERSION 3.8)

add_library(
    Render
    RenderBackend/VulkanBackend/Instance/src/Instance.cpp
    RenderBackend/VulkanBackend/Device/src/Device.cpp
    RenderBackend/VulkanBackend/Swapchain/src/Swapchain.cpp
    RenderBackend/VulkanBackend/Image/src/Image.cpp
    RenderBackend/VulkanBackend/VulkanRender/src/VulkanRender.cpp
    RenderBackend/VulkanBackend/RenderAttachment/src/RenderAttachment.cpp

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
    RenderBackend/VulkanBackend/RenderAttachment/include

    Window/include
    Logger/include
    Input/include

    Render/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/stb
)

add_library(Engine Window/src/Window.cpp
    Component/src/Component.cpp
    Input/src/Input.cpp
    Entity/src/Entity.cpp
    Scenegraph/src/Scenegraph.cpp
    defaultComponents/Transform/src/Transform.cpp
    ComponentRegistry/src/ComponentRegistry.cpp
    Eventsystem/src/Eventsystem.cpp
    JobSystem/src/JobSystem.cpp
)

target_include_directories(
    Engine PUBLIC Window/include
    Component/include
    Entity/include
    Scenegraph/include
    defaultComponents/Transform/include
    ComponentRegistry/include
    Threadpool/include
    Eventsystem/include
    Logger/include
    Input/include
    JobSystem/include
    Containers/include
)

add_library(Application Application/src/Application.cpp
)

target_link_libraries(Application PUBLIC Engine Render)

target_include_directories(Application PUBLIC Application/include)