#pragma once
#ifndef PIPELINE_BUILDER
#define PIPELINE_BUILDER
#include "Pipeline/Pipeline.h"
#include "CanellaUtility/CanellaUtility.h"
#include <json.hpp>
#include <fstream>
namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {

            class PipelineBuilder
            {
            public:
                static Pipeline BuildPipeline(const char *config)
                {

                    std::fstream f(config);
                    f;
                    nlohmann::json pipelineConfig;
                    f >> pipelineConfig;
                }

            private:
                std::vector<DescriptorSetLayout> createLayout(nlohmann::json &resources)
                {
                    uint32_t resourceCount = resources["ResourcesCount"].get<std::uint32_t>();
                    std::vector<ShaderBindingResource> bindingResources(resourceCount);
                    for (uint32_t i = 0; i < resourceCount; i++)
                    {
                        ShaderBindingResource bindingResource{};
                        bindingResource.type = coonvert_from_string_shader_resource_type(resources["Type"].get<std::string>().c_str());
                    }
                }

                inline std::vector<char> readFile(const std::string &filename)
                {
                    std::ifstream file(filename, std::ios::ate | std::ios::binary);
                    if (!file.is_open())
                    {
                        throw std::runtime_error("failed to open file!");
                    }

                    size_t fileSize = (size_t)file.tellg();
                    std::vector<char> buffer(fileSize);
                    file.seekg(0);
                    file.read(buffer.data(), fileSize);
                    file.close();

                    return buffer;
                }
            };
        }
    }
}

#endif