#include "RenderNodes/RenderNodes.h"

Canella::Rendersystem::VulkanBackend::MeshletGBufferPass::MeshletGBufferPass(
        Canella::Rendersystem::VulkanBackend::NodeType nodeType, const std::string &name,
        const Canella::Rendersystem::VulkanBackend::ResourcesRef &inputs,
        const Canella::Rendersystem::VulkanBackend::ResourcesRef &outputs,
        const Canella::Rendersystem::VulkanBackend::ResourcesRef &transient) : RenderNode(nodeType,
                                                                                          name,
                                                                                          inputs,
                                                                                          outputs,
                                                                                          transient) {}

void Canella::Rendersystem::VulkanBackend::MeshletGBufferPass::execute(Canella::Render &render)
{
}

void Canella::Rendersystem::VulkanBackend::MeshletGBufferPass::load_transient_resources(
                                                                                        nlohmann::json&,
                                                                                        Canella::Render &render)
{
    const auto& drawables = render.get_drawables();
    meshlets.resize(drawables.size());
    auto i = 0 ;
    for(const auto& drawable : drawables)
    {
        for(const auto& mesh_data : drawable.meshes)
            load_meshlet(meshlets[i], mesh_data);
        i++;
    }
}
