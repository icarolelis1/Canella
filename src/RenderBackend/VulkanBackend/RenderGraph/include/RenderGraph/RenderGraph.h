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

/**
 * THIS IS A HUGE TOPIC AND WILL INCREASE EXPONENTIALLY ON THE GO
 */
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

            using ResourcesRef =  std::vector<RefGPUResource>;
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

                RenderNode() = default;
                RenderNode(const std::string&,NodeType);
                RenderNode(const RenderNode& other) = delete;
                ~RenderNode() = default;

                virtual void execute(Canella::Render*, VkCommandBuffer, int ) ;
                //Give the node the resource loading logic
                virtual void load_transient_resources(nlohmann::json&, Canella::Render*);
                //Parse the json file with the configuration for each node
                void load_render_node(const nlohmann::json &) ;
                //write the outputs
                virtual void write_outputs();
                std::vector<std::weak_ptr<RenderNode>> descedent_nodes;

                NodeType type;
            protected:
                ResourcesRef inputs;
                ResourcesRef outputs;
                ResourcesRef transients;

                bool begin_render_pass = false;
                bool end_render_pass = false;

                std::shared_ptr<RenderNode> parent = nullptr;

                //Name of the render node that produces the input for this node
                std::string procuder_name;
                std::string renderpass_name;
                std::string pipeline_name;
                std::string pipeline_layout_name;
                bool final_output = false;
                std::string name;
			};

            using RefRenderNode =  std::shared_ptr<RenderNode>;

            /**
            * \brief Graph that contains all the render passes
            */
			class RenderGraph{
            public:
                RenderGraph();
                RenderGraph(const RenderGraph&other) = delete;
                static NodeType convert_from_string(const std::string&);

                ~RenderGraph() = default;

                void load_render_graph(const char* );
                void execute(VkCommandBuffer,Canella::Render*,int);
                void execute_descendent(const RefRenderNode&,VkCommandBuffer,Canella::Render*,int);
                void load_render_node(const nlohmann::json&,const RefRenderNode&);

            private:
                RefRenderNode start;
            };
		}
	}
}
#endif