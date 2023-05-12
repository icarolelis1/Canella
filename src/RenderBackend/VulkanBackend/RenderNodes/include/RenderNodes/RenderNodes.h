#pragma once
#ifndef RENDER_NODES
#define RENDER_NODES
#include "RenderGraph/RenderGraph.h"

namespace Canella {
    namespace Rendersystem {
        namespace VulkanBackend {

            struct MeshletGBufferPass : RenderNode{

                MeshletGBufferPass(NodeType nodeType, const std::string &name, const ResourcesRef &inputs,
                                   const ResourcesRef &outputs, const ResourcesRef &transient);

                void load_transient_resources(nlohmann::json&,Canella::Render &render) override;
                void execute(Canella::Render &render) override;
            private:
                std::vector<Canella::Meshlet> meshlets;

            };
        }
    }
}


#endif