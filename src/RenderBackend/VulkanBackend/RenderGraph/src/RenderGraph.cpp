#include "RenderGraph/RenderGraph.h"
#include "fstream"

void Canella::RenderSystem::VulkanBackend::RenderNode::load_render_node(const nlohmann::json& node_json)
{
    name = node_json["Name"].get<std::string>();
    pipeline_name = node_json["Pipeline"].get<std::string>();
    pipeline_layout_name = node_json["PipelineLayout"].get<std::string>();
    renderpass_name = node_json["Renderpass"].get<std::string>();
    final_output = node_json["OutPutToSwapchain"].get<bool>();
    end_render_pass = node_json["EndRenderPass"].get<bool>();
}

void Canella::RenderSystem::VulkanBackend::RenderNode::execute(Canella::Render *, VkCommandBuffer, int) {}

void Canella::RenderSystem::VulkanBackend::RenderNode::load_transient_resources(nlohmann::json &, Canella::Render *) {}

void Canella::RenderSystem::VulkanBackend::RenderNode::write_outputs() {}

Canella::RenderSystem::VulkanBackend::RenderNode::RenderNode(const std::string &node_name,
                                                             Canella::RenderSystem::VulkanBackend::NodeType node_type)
                                                             :
                                                             name(node_name),
                                                             type(node_type)
{

}

Canella::RenderSystem::VulkanBackend::RenderGraph::RenderGraph()
{
    start = std::make_shared<RenderNode>("render_node_root",NodeType::Root);
}

void Canella::RenderSystem::VulkanBackend::RenderGraph::load_render_graph(
        const char* render_graph_src)
{
    // Read renderpath json
    std::fstream f(render_graph_src);
    nlohmann::json render_graph_json;
    f >> render_graph_json;

   const auto& entry  = render_graph_json["RenderGraph"]["Entry"];
    load_render_node(entry,start);
}


void Canella::RenderSystem::VulkanBackend::RenderGraph::load_render_node(const nlohmann::json & entry,
                                                                         const RefRenderNode &ref_node)
{
    //Load the renderGraph recursively from the json file.
    for(const auto& descendent : entry["Descendents"])
    {
        auto render_node = std::make_shared<RenderNode>();
        //Deserialize the render_node data from the json file
        render_node->load_render_node(descendent);
        //add to the descendent vector
        ref_node->descedent_nodes.push_back(render_node);
        //Repeate the process recursively looking into Descendents descendents.
       // load_render_node(entry["Descendents"]["Descendents"],render_node);
    }
    int nb = 9;
}


Canella::RenderSystem::VulkanBackend::NodeType
Canella::RenderSystem::VulkanBackend::RenderGraph::convert_from_string(const std::string& type) {

    if (type == "Render")
        return NodeType::Render;
    return NodeType::Compute;

}

void Canella::RenderSystem::VulkanBackend::RenderGraph::execute(
                                                                    VkCommandBuffer commandBuffer,
                                                                    Canella::Render *render,
                                                                    int image_index)
{
    execute_descendent(start,commandBuffer,render,image_index);
}

void Canella::RenderSystem::VulkanBackend::RenderGraph::execute_descendent(
                                                                    const RefRenderNode& node,
                                                                    VkCommandBuffer command,
                                                                    Canella::Render * render,
                                                                    int image_index)
{

    for(const auto& descendent : node->descedent_nodes){
        auto locked = descendent.lock();
        if(!locked) continue;
        locked->execute(render,command,image_index);
        execute_descendent(locked,command,render,image_index);
    }
}
