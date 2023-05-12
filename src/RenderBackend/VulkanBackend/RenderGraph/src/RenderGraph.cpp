#include "RenderGraph/RenderGraph.h"

Canella::Rendersystem::VulkanBackend::RenderNode::RenderNode(NodeType nodeType, const std::string& _name,
                                                             ResourcesRef _inputs,
                                                             ResourcesRef _outputs,
                                                             ResourcesRef _independent) :
                                                             type(nodeType),
                                                             name(name),
                                                             inputs(_inputs),
                                                             outputs(_outputs),
                                                             transients(_independent)
{
}

Canella::Rendersystem::VulkanBackend::RenderNode::~RenderNode() {
    for (auto &resource: transients)
    {
        resource.reset();
    }
}

void Canella::Rendersystem::VulkanBackend::RenderNode::load_render_node(nlohmann::json &){}

void Canella::Rendersystem::VulkanBackend::RenderNode::execute(Canella::Render& renderer) {}

void Canella::Rendersystem::VulkanBackend::RenderNode::load_transient_resources(nlohmann::json&,Canella::Render &) {}


Canella::Rendersystem::VulkanBackend::RenderGraph::RenderGraph()
{
    start = std::make_shared<RenderNode>(NodeType::Root,"render_node_root");
}

void Canella::Rendersystem::VulkanBackend::RenderGraph::load_render_graph(
        nlohmann::json &,
        Canella::Drawables &drawables)
{

}

