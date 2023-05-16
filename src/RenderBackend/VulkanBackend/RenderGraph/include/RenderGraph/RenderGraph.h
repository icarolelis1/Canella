#pragma once
#ifndef CANELLA_RENDER_GRAPH
#define CANELLA_RENDER_GRAPH
#include "Resources/Resources.h"
#include "Render/Render.h"
#include "Device/Device.h"
#include <memory>
#include <string>
#include <vector>
#include "json.hpp"
namespace Canella {
	namespace RenderSystem {
		namespace VulkanBackend {
            enum NodeType{
                Compute,
                Render,
                Root,
            };

			struct RenderNodeOutputs {
                std::vector<RenderSystem::VulkanBackend::GPUResource> resources;
			};

            struct RenderEdge{

            };

            using ResourceRef = std::shared_ptr<RenderNodeOutputs>;
            using ResourcesRef =  std::vector<ResourceRef>;
            /**
            * \brief A individual work unit of the render graph
            * \param nodeType type of node
            * \param name name of the node
            * \param inputs data used as inputs
            * \param outputs data that the node outputs
            * \param transient data only used in by this node
            */
			class RenderNode {
            public:

                RenderNode(NodeType nodeType,
                           const std::string& name,
                           ResourcesRef inputs = std::vector<ResourceRef>(),
                           ResourcesRef outputs = std::vector<ResourceRef>(),
                           ResourcesRef transient = std::vector<ResourceRef>() );

                RenderNode(const RenderNode& other) = delete;
                ~RenderNode();

                virtual void execute(Canella::Render*, VkCommandBuffer, int ) ;
                //Give the node the resource loading logic
                virtual void load_transient_resources(nlohmann::json&, Canella::Render*);
                //Parse the json file with the configuration for each node
                virtual void load_render_node(nlohmann::json&) ;
                NodeType type;
            protected:
                ResourcesRef inputs;
                ResourcesRef outputs;
                ResourcesRef transients;

                std::shared_ptr<RenderNode> parent = nullptr;
                std::vector<std::shared_ptr<RenderNode>> connected_nodes;

                //Name of the render node that produces the input for this node
                const std::string procuder_name;
				const std::string renderpass;
				const std::string pipeline;
                bool final_output = false;
				const std::string name;
			};

            using RefRenderNode =  std::shared_ptr<RenderNode>;

            /**
            * \brief Graph that contains all the render passes
            */
			class RenderGraph{
            public:
                RenderGraph();
                RenderGraph(const RenderGraph&other) = delete;
                ~RenderGraph() = default;

                void load_render_graph(nlohmann::json&,Drawables& drawables);

            private:
                RefRenderNode start;
            };
		}
	}
}
#endif