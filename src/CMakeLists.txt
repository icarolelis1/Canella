cmake_minimum_required(VERSION 3.8)

add_library(
    Render
    Stb/stb.h
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
    RenderBackend/VulkanBackend/RenderNodes/src/RenderNodes.cpp
    RenderBackend/VulkanBackend/PipelineBuilder/src/PipelineBuilder.cpp
    RenderBackend/VulkanBackend/Frame/src/Frame.cpp
    RenderBackend/VulkanBackend/CanellaUtility/src/CanellaUtility.cpp
    RenderBackend/VulkanBackend/Pipeline/src/Pipeline.cpp
    RenderBackend/VulkanBackend/DescriptorSet/src/DescriptorSet.cpp
    RenderBackend/VulkanBackend/RenderGraph/src/RenderGraph.cpp


    Render/src/Render.cpp
    SpirV/spirv_reflect.c
)

target_include_directories(
    Render PUBLIC
    RenderBackend/VulkanBackend/Instance/include
    RenderBackend/VulkanBackend/Device/include
    RenderBackend/VulkanBackend/Commandpool/include
    RenderBackend/VulkanBackend/Swapchain/include
    RenderBackend/VulkanBackend/Image/include
    RenderBackend/VulkanBackend/VulkanRender/include
    RenderBackend/VulkanBackend/RenderAttachment/include
    RenderBackend/VulkanBackend/RenderpassManager/include
    RenderBackend/VulkanBackend/Resources/include
    RenderBackend/VulkanBackend/PipelineBuilder/include
    RenderBackend/VulkanBackend/RenderNodes/include
    RenderBackend/VulkanBackend/Renderpass/include
    RenderBackend/VulkanBackend/Frame/include
    RenderBackend/VulkanBackend/CanellaUtility/include
    RenderBackend/VulkanBackend/Pipeline/include
    RenderBackend/VulkanBackend/Descriptorpool/include
    RenderBackend/VulkanBackend/DescriptorSet/include
    RenderBackend/VulkanBackend/RenderGraph/include
    SpirV/
    Window/include
    Logger/include
    Input/include
    Render/include
    Meshoptimizer 
     Meshoptimizer/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../dependencies/stb
)

add_library(
    Systems 
    Window/src/Window.cpp
    Input/src/Input.cpp
    Eventsystem/src/Eventsystem.cpp
    JobSystem/src/JobSystem.cpp
)

target_include_directories(
    Systems 
    PUBLIC Window/include
    Threadpool/include
    Eventsystem/include
    Logger/include
    Input/include
    JobSystem/include
    Containers/include
)

file (GLOB MESHOPT_SRC
    "Meshoptimizer/src/*.cpp"
)
add_library(
   Meshoptimizer
   ${MESHOPT_SRC}

)

target_include_directories(Meshoptimizer 
PUBLIC Meshoptimizer/include)
target_link_libraries(Render PUBLIC Meshoptimizer)

add_library(
Engine 
Project/src/Project.cpp     
Components/src/Components.cpp
Scene/src/Scene.cpp
Entity/src/Entity.cpp
Systems/src/Systems.cpp
Serializer/src/Serializer.cpp
)
target_include_directories(Engine PUBLIC 
Entity/include
Project/include    
Components/include
Systems/include
Scene/include
Serializer/include
)

target_link_libraries(Engine PUBLIC Render Systems)

