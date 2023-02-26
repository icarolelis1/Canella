cmake_minimum_required(VERSION 3.8)

add_library(
    Render
    RenderBackend/VulkanBackend/Instance/src/Instance.cpp
    RenderBackend/VulkanBackend/Device/src/Device.cpp
    RenderBackend/VulkanBackend/Commandpool/src/Commandpool.cpp
    RenderBackend/VulkanBackend/Swapchain/src/Swapchain.cpp
    RenderBackend/VulkanBackend/Image/src/Image.cpp
    RenderBackend/VulkanBackend/VulkanRender/src/VulkanRender.cpp
    RenderBackend/VulkanBackend/RenderAttachment/src/RenderAttachment.cpp
    RenderBackend/VulkanBackend/RenderpassManager/src/RenderpassManager.cpp
    RenderBackend/VulkanBackend/Renderpass/src/Renderpass.cpp
    RenderBackend/VulkanBackend/Resources/src/Resources.cpp
    RenderBackend/VulkanBackend/Descriptorpool/src/Descriptorpool.cpp
    RenderBackend/VulkanBackend/PipelineBuilder/src/PipelineBuilder.cpp
    RenderBackend/VulkanBackend/Frame/src/Frame.cpp
    RenderBackend/VulkanBackend/CanellaUtility/src/CanellaUtility.cpp
    RenderBackend/VulkanBackend/Pipeline/src/Pipeline.cpp


    Render/src/Render.cpp
    Stb/stb.h
    SpirV/spirv_reflect.c
)

target_include_directories(
    Render PUBLIC
    Stb/
    RenderBackend/VulkanBackend/Instance/include
    RenderBackend/VulkanBackend/Device/include
    RenderBackend/VulkanBackend/Commandpool/include
    RenderBackend/VulkanBackend/Swapchain/include
    RenderBackend/VulkanBackend/Image/include
    RenderBackend/VulkanBackend/VulkanRender/include
    RenderBackend/VulkanBackend/RenderAttachment/include
    RenderBackend/VulkanBackend/RenderpassManager/include
    RenderBackend/VulkanBackend/Resources/include
    RenderBackend/VulkanBackend/Renderpass/include
    RenderBackend/VulkanBackend/PipelineBuilder/include
    RenderBackend/VulkanBackend/Frame/include
    RenderBackend/VulkanBackend/CanellaUtility/include
    RenderBackend/VulkanBackend/Pipeline/include
    RenderBackend/VulkanBackend/Descriptorpool/include


    SpirV/

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
    defaultComponents/CameraController/src/CameraController.cpp
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
    defaultComponents/CameraController/include
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