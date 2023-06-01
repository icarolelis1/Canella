#include "CanellaUtility/CanellaUtility.h"
#include <vector>
#include <numeric>
#include <meshoptimizer.h>

namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            void VK_CHECK(VkResult result,const char* message){
                if(result != VK_SUCCESS)
                    throw std::runtime_error("message");
            }

            VkFormat convert_from_string_format(const char* image_format)
            {
                if(strcmp(image_format,"VK_FORMAT_B8G8R8A8_UNORM") == 0)
                    return VK_FORMAT_B8G8R8A8_UNORM;
            }

            VkSampleCountFlagBits convert_from_string_sample_count(const char* samples)
            {
                return VK_SAMPLE_COUNT_1_BIT;
            }

            VkAttachmentLoadOp convert_from_string_loadOp(const char* loadOp)
            {
                if(strcmp(loadOp,"VK_ATTACHMENT_LOAD_OP_LOAD") == 0)
                    return VK_ATTACHMENT_LOAD_OP_LOAD;
                return VK_ATTACHMENT_LOAD_OP_CLEAR;
            }

            VkAttachmentStoreOp convert_from_string_storeOp(const char* storeOp)
            {
                if (strcmp(storeOp, "VK_ATTACHMENT_STORE_OP_STORE") == 0)
                    return VK_ATTACHMENT_STORE_OP_STORE;
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }

            VkImageLayout convert_from_string_image_layout(const char* layout)
            {
                if (strcmp(layout, "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR") == 0)
                    return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                if(strcmp(layout,"VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL") == 0)
                    return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                if(strcmp(layout,"VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL") == 0)
                    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                if(strcmp(layout,"VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL") == 0)
                    return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                return VK_IMAGE_LAYOUT_UNDEFINED;
            }

            VkPipelineStageFlagBits convert_from_string_pipeline_stage(const char* stage)
            {
                return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            }

            VkAccessFlagBits convert_from_string_access_mask(const char* mask)
            {
                if (strcmp(mask, "VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT") == 0)
                    return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            }

            ShaderResourceType convert_from_string_shader_resource_type(const char* type)
            {
                if (strcmp(type, "UNIFORM_BUFFER") == 0)
                    return ShaderResourceType::UNIFORM_BUFFER;
                if (strcmp(type, "STORAGE_BUFFER") == 0)
                    return ShaderResourceType::STORAGE_BUFFER;
                return ShaderResourceType::STORAGE_BUFFER;

            }

            VkShaderStageFlagBits convert_from_string_shader_stage(const char* stage)
            {

                if (strcmp(stage, "MESH_STAGE") == 0)
                    return VK_SHADER_STAGE_MESH_BIT_EXT;
                else if (strcmp(stage, "TASK_STAGE") == 0)
                    return VK_SHADER_STAGE_TASK_BIT_EXT;
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            }

            VkShaderStageFlags read_shader_stage_from_json(nlohmann::json stages_json) {
                std::vector<VkShaderStageFlagBits> shader_stages;
                std::vector<std::string>stages = stages_json.get<std::vector<std::string>>();
                for (auto& stage : stages)
                    shader_stages.push_back(convert_from_string_shader_stage(stage.c_str()));

                VkShaderStageFlagBits stages_mask = shader_stages[0];
            
                //Todo implement this. I have no idea how to do this lol
                if (shader_stages.size() > 1)
                    return VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
                return stages_mask;
            }
            size_t get_size_of_structure(const char* structure)
            {
                if(strcmp(structure,"ViewProjection"))
                    return sizeof(ViewProjection);
                if (strcmp(structure, "Meshlet"))
                    return sizeof(meshopt_Meshlet);
                return sizeof(meshopt_Meshlet);
            }

            uint64_t uuid() {
                return uniform_distribution(random_engine);
            }

            void create_render_query(RenderQueries &renderQueries, Device *device) {
                VkQueryPoolCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
                info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
                info.pipelineStatistics =
                        VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
                        VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                        VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
                        VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
                        VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
                        VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;
                info.queryCount = 1;
                renderQueries.statistics.resize(1);

                VkQueryPoolCreateInfo timeInfo{};
                timeInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
                timeInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
                timeInfo.queryCount = 2;
                renderQueries.time_stamps.resize(2);
                VK_CHECK(vkCreateQueryPool(device->getLogicalDevice(),
                                           &timeInfo,
                                           device->getAllocator(),
                                           & renderQueries.timestamp_pool),
                         "Failed to create timestamp pool");
            }


        }
    };
}