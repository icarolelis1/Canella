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
            class GeometryPass : public RenderNode
            {
            public:
                struct IndirectCommandToCull
                {
                    uint32_t groupCountX;
                    uint32_t groupCountY;
                    uint32_t groupCountZ;
                    uint32_t draw_id;
                };

                struct alignas(16) CullingData
                {
                    glm::vec4 frustumPlanes[6];
                    glm::vec4 coefficients_width_znear;
                    glm::vec4 width_height_padding;
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

                struct HIZDepth
                {
                    ResourceAccessor pyramidImage;
                    std::array<VkImageView,16> mip_views;
                    uint32_t mip_count;
                    bool visibility_first_cleared = false;
                    uint32_t base_width;
                    uint32_t base_height;
                };

                GeometryPass();
                ~GeometryPass();
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
                void write_descriptorsets_geometry( Canella::Render *render);
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

                /**
                 * @brief Render to the HIZ chain
                 * @param render
                 * @param command
                 */
                void update_hiz_chain(Canella::Render *render,
                                      VkCommandBuffer &command,
                                      int image_index);


                //Temp
                //To do remove this from here
                void create_push_descriptor(Canella::Render* render);

                /**
                 * @brief Builds a hierarchical depth of mips to be used in  occlusion  culling
                 * @params width Frame Width
                 * @params height Frame Height
                 */
                void build_hierarchical_depth(Canella::Render* render);

                std::vector<ResourceAccessor> occlusion_visibility_buffer;
                std::vector<ResourceAccessor> resource_meshlet_buffers;
                std::vector<ResourceAccessor> resource_meshlet_triangles;
                std::vector<ResourceAccessor> resource_meshlet_vertices;
                std::vector<ResourceAccessor> resource_vertices_buffers;
                std::vector<ResourceAccessor> resource_bounds_buffers;
                std::vector<ResourceAccessor> draw_indirect_buffers;
                std::vector<ResourceAccessor>      command_count_buffers;
                std::vector<DescriptorPerDrawable> geometry_data_descriptors;
                std::vector<DescriptorPerDrawable> descriptors;
                std::vector<ResourceAccessor> resource_static_meshes;

                // debug commands
                std::vector<IndirectCommandToCull> commands;
                HIZDepth hiz_depth;
                RenderQueries queries;
                bool post_first_load = false;
                Device *device;
            };
        }
    }
}

#endif