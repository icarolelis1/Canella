#pragma once
#ifndef PIPELINE_BUILDER
#define PIPELINE_BUILDER
#include "Pipeline/Pipeline.h"
#include "RenderpassManager/RenderpassManager.h"
#include "CanellaUtility/CanellaUtility.h"
#include <json.hpp>
#include <unordered_map>
#include <fstream>
namespace Canella
{
    namespace RenderSystem
    {
        namespace VulkanBackend
        {
            using DescriptorSetLayouts = std::unordered_map<const char *, DescriptorSetLayout>;
            using PipelineLayouts = std::unordered_map<const char *, PipelineLayout>;
            using Pipelines = std::unordered_map<const char *, Pipeline>;

            class PipelineBuilder
            {
            public:
                /**
                 * @brief
                 * @param pipelineData json metadata containing information about the pipelines
                 * @param descriptor_set_layouts map structure for caching DescripotrsetLayouts
                 * @param pipeline_layouts map structure for caching PipelineLayouts
                 * @param pipelines map structure for caching Pipelines
                 */
                static void cachePipelineData(Device *device,
                    nlohmann::json &pipelineData,
                    DescriptorSetLayouts &descriptor_set_layouts,
                    PipelineLayouts &pipeline_layouts,
                    Pipelines &pipelines)
                {
                    createPipelineLayouts(device,descriptor_set_layouts,pipeline_layouts,)
                }

                /*
                      static Pipeline BuildPipeline(Device *device,
                                                    RenderpassManager &renderpasses,
                                                    const char *config,
                                                    std::vector<DescriptorSetLayout> descriptorSetLayouts,
                                                    std::vector<VkPushConstantRange> pushConsts)
                      {

                          std::fstream f(config);
                          f;
                          nlohmann::json pipelineConfig;
                          f >> pipelineConfig;

                          PipelineLayout pipelineLayout = PipelineLayout();
                          pipelineLayout.build(device, descriptorSetLayouts, pushConsts);
                          auto shaderCount = pipelineConfig["ShaderCount"].get<std::uint32_t>();
                      }*/
            private:

                static DescriptorSetLayout createDescriptorSetLayout(Device *device, nlohmann::json &resources)
                {
                    uint32_t resourceCount = resources["ResourceCount"].get<std::uint32_t>();
                    std::vector<ShaderBindingResource> bindingResources(resourceCount);
                    for (uint32_t i = 0; i < resourceCount; i++)
                    {
                        nlohmann::json &bindings = resources["BindingResources"][i];
                        ShaderBindingResource bindingResource{};
                        bindingResource.type = convert_from_string_shader_resource_type(bindings["Type"].get<std::string>().c_str());
                        bindingResource.stages = convert_from_string_shader_stage(bindings["Stage"].get<std::string>().c_str());
                        bindingResource.size = sizeof(ViewProjection);
                        bindingResource.binding = bindings["Binding"].get<std::uint32_t>();
                        bindingResources[i] = bindingResource;
                    }
                    DescriptorSetLayout descriptor = DescriptorSetLayout();
                    descriptor.build(device, std::move(bindingResources));
                    return std::move(descriptor);
                }

                inline void cacheDescriptorSetLayouts(
                    Device *device,
                    nlohmann::json pipelineData,
                    std::vector<VkPushConstantRange> &pushConstants,
                    DescriptorSetLayouts &cachedDescriptorLayouts,
                    PipelineLayouts &cachedPipelineLayouts)
                {
                    const auto descriptorSetLayoutCount = pipelineData["PipelineData"]["DescriptorSetLayoutCounts"].get<std::uint32_t>();
                    std::vector<DescriptorSetLayout> descriptor_set_layouts(descriptorSetLayoutCount);

                    for (auto i = 0; i < descriptorSetLayoutCount; i++)
                        cacheDescriptorSetLayout(device, pipelineData, descriptor_set_layouts, cachedDescriptorLayouts, i);
                }

                inline void cacheDescriptorSetLayout(
                    Device *device,
                    nlohmann::json pipelineData,
                    std::vector<DescriptorSetLayout> &descriptor_set_layouts,
                    DescriptorSetLayouts &cachedDescriptorSetLayouts,
                    const int i)
                {
                    descriptor_set_layouts[i] = PipelineBuilder::createDescriptorSetLayout(device,
                                                                                           pipelineData["PipelineData"]["DescriptorSetLayouts"][i]);
                    const char *key = pipelineData["PipelineData"]["DescriptorSetLayouts"][i]["Key"].get<std::string>().c_str();
                    if (cachedDescriptorSetLayouts.find(key) == cachedDescriptorSetLayouts.end())
                        cachedDescriptorSetLayouts[key] = std::move(descriptor_set_layouts[i]);
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

                inline void createPipelineLayouts(
                    Device *device,
                    DescriptorSetLayouts &cachedDescriptorSetLayouts,
                    PipelineLayouts &cachedPipelineLayouts,
                    nlohmann::json& pipeline_data)
                {
                    // Cache DescriptorSetLayouts
                    std::vector<VkPushConstantRange> pushConstants;
                    cacheDescriptorSetLayouts(device,pipelineData, pushConstants,cachedDescriptorSetLayouts,cachedPipelineLayouts);
                    nlohmann::json pipelines_json = pipelineData["PipelineData"]["Pipelines"];
                    const auto number_of_pipelines = pipelineData["PipelineData"]["PipelinesCount"].get<std::uint32_t>();

                    // Cache Pipeline Layouts
                    for (auto i = 0; i < number_of_pipelines; i++)
                    {
                        nlohmann::json pipeline = pipelines_json[i];
                        auto number_of_descriptorset_layouts = pipeline["NumerOfDescriptorSets"].get<std::uint32_t>();
                        std::vector<DescriptorSetLayout> descriptor_set_layouts(number_of_descriptorset_layouts);
                        for (auto j = 0; j < number_of_descriptorset_layouts; j++)
                            descriptor_set_layouts.push_back(
                                cachedDescriptorSetLayouts[pipeline["PipelineLayoutDescriptors"][j].get<std::string>().c_str()]);
                        cachedPipelineLayouts[pipeline["Key"].get<std::string>().c_str()] = PipelineLayout();
                        cachedPipelineLayouts[pipeline["Key"].get<std::string>().c_str()].build(device, descriptor_set_layouts, pushConstants);
                    }

                    //
                    // pipelineLayouts["basic"] = PipelineLayout();
                    // pipelineLayouts["basic"].build(&device, descriptor_set_layouts, pushConstants);
                    //
                    // global_descriptors.resize(swapChain.getNumberOfImages());
                    // descriptorPool.AllocateDescriptorSet(device, descriptor_set_layouts[0], global_descriptors);
                    // descriptor_set_layouts[0].destroy(&device);
                }
            };
        }
    }
}

#endif