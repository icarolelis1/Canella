#pragma once
#ifndef RENDER_NODES
#define RENDER_NODES
#include "RenderGraph/RenderGraph.h"
#include "CanellaUtility/CanellaUtility.h"

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            class GeomtryPass : public RenderNode
            {
            public:
                struct IndirectCommandToCull
                {
                    uint32_t groupCountX;
                    uint32_t groupCountY;
                    uint32_t groupCountZ;
                    uint32_t draw_id;
                };

                struct alignas(16) StaticMeshData
                {
                    glm::vec3 center;
                    float radius;
                    uint32_t mesh_id;
                    uint32_t vertex_offset;
                    uint32_t index_offset;
                    uint32_t meshlet_offset;
                    uint32_t meshlet_vertices_offset;
                    uint32_t meshlet_triangles_offset;
                    uint32_t meshlet_count;
                };

                struct DescriptorPerDrawable
                {
                    std::vector<VkDescriptorSet> descriptor_sets;
                    std::vector<VkDescriptorSet> vertex_descriptorset;
                    std::vector<VkDescriptorSet> indices_descriptorset;
                };

                GeomtryPass();
                ~GeomtryPass();
                void execute(Canella::Render *render, VkCommandBuffer &, int) override;
                void load_transient_resources(Canella::Render *render) override;

            private:
                /**
                 * @brief creates the resource used by this node
                 * @param render Application Renderer
                 */
                void create_resource_buffers(Canella::Render *render);
                /**
                 * @brief Creates the indirect command for IndirectDrawing
                 * @param render
                 */
                void create_indirect_commands(Canella::Render *render);
                /**
                 * Writes the Descriptorset
                 * @param render
                 */
                void write_descriptorsets_geomtry(Canella::Render *render);
                /**
                 * @brief Writes the descriptorsets used in the culling pass
                 * @param render
                 */
                void write_descriptorsets_culling(Canella::Render *render);

                /**
                 * @brief writes the outputs to subsequent nodes
                 */
                void write_outputs() override;
                /**
                 * @brief Bind the resize window event (Resources needs to be recreated)
                 * @param render Application Renderer
                 */
                void setup_reload_resource_event(Canella::Render *render);
                /**
                 * @brief Clean the resources associated with the node
                 * @param render Application Renderer
                 */
                void clear_render_node(Canella::Render *render);

                /**
                 * @brief performs Frustum Culling in a compute shader
                 * @param Application Renderer
                 * @param command Command buffer used in Dispatch
                 * @param compute_pipeline compute pipeline that executes frustum culling
                 * @param drawables objects to process culling
                 * @param image_index frame index
                 */
                void compute_frustum_culling(Canella::Render *render,
                                             VkCommandBuffer &command,
                                             VkPipeline &compute_pipeline,
                                             Drawables &drawables,
                                             int image_index);

                std::vector<ResourceAccessor> resource_meshlet_buffers;
                std::vector<ResourceAccessor> resource_meshlet_triangles;
                std::vector<ResourceAccessor> resource_meshlet_vertices;
                std::vector<ResourceAccessor> resource_vertices_buffers;
                std::vector<ResourceAccessor> resource_bounds_buffers;
                std::vector<ResourceAccessor> draw_indirect_buffers;
                std::vector<ResourceAccessor> command_count_buffers;
                std::vector<DescriptorPerDrawable> frustum_culling_descriptors;
                std::vector<DescriptorPerDrawable> descriptors;
                std::vector<ResourceAccessor> resource_static_meshes;
                
                // debug commands
                std::vector<IndirectCommandToCull> commands;

                RenderQueries queries;
                bool post_first_load = false;
                Device *device;
            };
        }
    }
}

#endif