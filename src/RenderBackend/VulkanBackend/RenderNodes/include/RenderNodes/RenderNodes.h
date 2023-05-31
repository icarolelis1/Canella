#pragma once
#ifndef RENDER_NODES
#define RENDER_NODES
#include "RenderGraph/RenderGraph.h"
#include "CanellaUtility/CanellaUtility.h"

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
                ~MeshletGBufferPass();
                void load_transient_resources(Canella::Render *render) override;
                void execute(Canella::Render *render,VkCommandBuffer,int) override;
                void write_outputs() override;

            private:
                std::vector<ResourceAccessor> resource_meshlet_buffers;
                std::vector<ResourceAccessor> resource_meshlet_triangles;
                std::vector<ResourceAccessor> resource_meshlet_vertices;
                std::vector<ResourceAccessor> resource_vertices_buffers;
                std::vector<ResourceAccessor> resource_bounds_buffers;
                std::vector<DescriptorsPerImage> descriptors;
                std::vector<Canella::Meshlet> meshlets;
                RenderQueries queries;
                void setup_reload_resource_event(Canella::Render*);
                void clear_render_node(Canella::Render *render);
                bool debug_statics = true;
                bool post_first_load = false;
                Device* device;
            };
        }
    }
}


#endif