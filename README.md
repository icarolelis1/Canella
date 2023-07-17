# Canella

Experiments on crafting a 3D vulkan game engine.
(Work In Progress)

# Feature List
- Task and mesh shader implementation and early backface culling.  Mesh is decomposed of a collection of small portions called meshlets. The task shader filter the meshes by using backface culling

[![IMAGE ALT TEXT](http://img.youtube.com/vi/gYv5ZlkNnf8/0.jpg)](http://www.youtube.com/watch?v=gYv5ZlkNnf8 "Cluster Visualization")

- Frustum Culling in compute Shaders
  
[![IMAGE ALT TEXT](http://img.youtube.com/vi/1L7Vubym8GA/0.jpg)](http://www.youtube.com/watch?v=1L7Vubym8GA "Frustum Culling")

- ImGui Implementation
- GPU Driven Indirect Drawing.
- Graphic setup serialization (Creation of Renderpass and Pipelines) in json FORMAT.
- RenderGraph to automatically create the render passes and write attachments (WORK IN PROGRESS).

### Features To be done
- Virtualized Texture Access.
- Vertex Skinning and IK Rig
- Implement more render nodes (PBR path with indirect diffuse, HBAO+,Bloom etc...)
- Multhread command buffer generation
- and the list will go on forever...

# Dependencies
- Vulkan SDK - https://vulkan.lunarg.com/doc/view/latest/windows/getting_started.html
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

