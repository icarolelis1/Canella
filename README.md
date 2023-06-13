# Canella

Experiments on crafting a 3D vulkan game engine.
(Work In Progress)


![image](https://github.com/icarolelis1/Canella/assets/27213125/56cd7068-2a14-483a-bcb1-d35dfc4a8cc3)


# Feature List
- Task and mesh shader implementation and early backface culling.  Mesh is decomposed of a collection of small portions called meshlets. The task shader filter the meshes by using backface culling
- ImGui Implementation
- Graphic setup serialization (Creation of Renderpass and Pipelines) in json FORMAT.
- RenderGraph to automatically create the render passes and write attachments (WORK IN PROGRESS).

### Features To be done
- Use DrawMeshtaksIndirect and more advanced culling (Frustum Culling and triangle culling)
- Vertex Skinning and
- Implement more render nodes (PBR path with indirect diffuse, HBAO+,Bloom etc...)
- Multhread command buffer generation
- and the list will go on forever...

### Dependencies
- Vulkan SDK - https://vulkan.lunarg.com/doc/view/latest/windows/getting_started.html

### External Libraries
- Cmake https://cmake.org 
- ImGui https://github.com/ocornut/imgui
- Meshoptmizer https://github.com/zeux/meshoptimizer
- glfw https://www.glfw.org
- glm https://github.com/g-truc/glm

# Build
clone the project 
```
git clone https://github.com/icarolelis1/Canella.git
mkdir build
cd build
cmake ..
```

### Currently not building on Linux.
