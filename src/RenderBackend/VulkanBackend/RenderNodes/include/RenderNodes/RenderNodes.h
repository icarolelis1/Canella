#pragma once
#ifndef RENDER_NODES
#define RENDER_NODES
#include "RenderGraph/RenderGraph.h"

namespace Canella {
    namespace RenderSystem {
        namespace VulkanBackend {

            struct MeshletGBufferPass :public RenderNode{
                typedef struct DescriptorsPerImage{
                    std::vector<VkDescriptorSet> descriptor_sets;
                    std::vector<VkDescriptorSet> vertex_descriptorset;
                    std::vector<VkDescriptorSet> indices_descriptorset;
                };

                MeshletGBufferPass() = default;

                void load_transient_resources(Canella::Render *render) override;
                void execute(Canella::Render *render,VkCommandBuffer,int) override;
                void write_outputs() override;
            private:
                std::vector<ResourceAccessor> resource_meshlet_buffers;
                std::vector<ResourceAccessor> resource_vertices_buffers;
                std::vector<ResourceAccessor> resource_indices_buffers;
                std::vector<DescriptorsPerImage> descriptors;
                std::vector<Canella::Meshlet> meshlets;

            };
        }
    }
}


#endif