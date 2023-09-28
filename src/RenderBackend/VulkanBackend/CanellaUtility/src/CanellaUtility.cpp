#include "CanellaUtility/CanellaUtility.h"
#include <vector>
#include <numeric>
#include <meshoptimizer.h>

namespace Canella
{
    uint64_t uuid()
    {
        return uniform_distribution(random_engine);
    }

    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            void VK_CHECK(VkResult result, const char *message)
            {
                if (result != VK_SUCCESS)
                    throw std::runtime_error("message");
            }

            VkFormat convert_from_string_format(const char *image_format)
            {
                if (strcmp(image_format, "VK_FORMAT_B8G8R8A8_UNORM") == 0)
                    return VK_FORMAT_B8G8R8A8_UNORM;
                else if(strcmp(image_format, "VK_FORMAT_R8G8B8A8_UNORM") == 0)
                    return VK_FORMAT_R8G8B8A8_UNORM;
                else if(strcmp(image_format, "VK_FORMAT_R16G16_SFLOAT") == 0)
                    return VK_FORMAT_R16G16_SFLOAT;
                else if(strcmp(image_format, "VK_FORMAT_R16G16B16A16_SFLOAT") == 0)
                    return VK_FORMAT_R16G16B16A16_SFLOAT;

                return VK_FORMAT_B8G8R8A8_UNORM;
            }

            VkSampleCountFlagBits convert_from_string_sample_count(const char *samples)
            {
                return VK_SAMPLE_COUNT_1_BIT;
            }

            VkAttachmentLoadOp convert_from_string_loadOp(const char *loadOp)
            {
                if (strcmp(loadOp, "VK_ATTACHMENT_LOAD_OP_LOAD") == 0 )
                    return VK_ATTACHMENT_LOAD_OP_LOAD;
                if (strcmp(loadOp, "VK_ATTACHMENT_LOAD_OP_DONT_CARE") == 0 )
                    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                return VK_ATTACHMENT_LOAD_OP_CLEAR;
            }

            VkAttachmentStoreOp convert_from_string_storeOp(const char *storeOp)
            {
                if (strcmp(storeOp, "VK_ATTACHMENT_STORE_OP_STORE") == 0)
                    return VK_ATTACHMENT_STORE_OP_STORE;
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            }

            VkImageLayout convert_from_string_image_layout(const char *layout)
            {
                if (strcmp(layout, "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR") == 0)
                    return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                else if (strcmp(layout, "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL") == 0)
                    return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                else  if (strcmp(layout, "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL") == 0)
                    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                else if (strcmp(layout, "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL") == 0)
                    return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                else if (strcmp(layout, "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL") == 0)
                    return  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                return VK_IMAGE_LAYOUT_UNDEFINED;
            }

            VkPipelineStageFlagBits convert_from_string_pipeline_stage(const char *stage)
            {
                return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            }

            VkAccessFlagBits convert_from_string_access_mask(const char *mask)
            {
                if (strcmp(mask, "VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT") == 0)
                    return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            }

            ShaderResourceType convert_from_string_shader_resource_type(const char *type)
            {
                if (strcmp(type, "UNIFORM_BUFFER") == 0)
                    return ShaderResourceType::UNIFORM_BUFFER;
                else if (strcmp(type, "STORAGE_BUFFER") == 0)
                    return ShaderResourceType::STORAGE_BUFFER;
                else if (strcmp(type, "SAMPLER_2D") == 0)
                    return ShaderResourceType::IMAGE_SAMPLER;
                else if (strcmp(type, "IMAGE_STORAGE") == 0)
                    return ShaderResourceType::IMAGE_STORAGE;
                else if (strcmp(type, "SAMPLER_CUBE") == 0)
                    return ShaderResourceType::IMAGE_SAMPLER_CUBE;
                return ShaderResourceType::STORAGE_BUFFER;
            }

            VkShaderStageFlagBits convert_from_string_shader_stage(const char *stage)
            {
                if (strcmp(stage, "MESH_STAGE") == 0)
                    return VK_SHADER_STAGE_MESH_BIT_EXT;
                else if (strcmp(stage, "TASK_STAGE") == 0)
                    return VK_SHADER_STAGE_TASK_BIT_EXT;
                else if (strcmp(stage, "COMPUTE_STAGE") == 0)
                    return VK_SHADER_STAGE_COMPUTE_BIT;
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            }

            VkShaderStageFlags read_shader_stage_from_json(nlohmann::json stages_json)
            {
                std::vector<std::string> stages = stages_json.get<std::vector<std::string>>();
                VkShaderStageFlags first_stage= convert_from_string_shader_stage(stages[0].c_str());
                for (auto i = 1; i < stages.size(); i++)
                    first_stage |= convert_from_string_shader_stage(stages[i].c_str());;
                return first_stage;
            }

            size_t get_size_of_structure(const char *structure)
            {
                if (strcmp(structure, "ViewProjection"))
                    return sizeof(ViewProjection);
                if (strcmp(structure, "Meshlet"))
                    return sizeof(meshopt_Meshlet);
                return sizeof(meshopt_Meshlet);
            }

            void create_render_query(RenderQueries &renderQueries, Device *device,uint32_t time_stamps_count)
            {
                VkQueryPoolCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
                info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
                info.pipelineStatistics =
                        VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT;
                        VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT |
                        VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT;

                info.queryCount = 3;
                renderQueries.statistics.resize(3);
                VK_CHECK(vkCreateQueryPool(device->getLogicalDevice(),&info,device->getAllocator(),&renderQueries.statistics_pool),"Failed to create timestamp pool");

                VkQueryPoolCreateInfo timeInfo{};
                timeInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
                timeInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
                timeInfo.queryCount = time_stamps_count;
                renderQueries.time_stamps.resize(time_stamps_count);
                VK_CHECK(vkCreateQueryPool(device->getLogicalDevice(),&timeInfo,device->getAllocator(),&renderQueries.timestamp_pool),"Failed to create timestamp pool");
            }
        }
    };
}