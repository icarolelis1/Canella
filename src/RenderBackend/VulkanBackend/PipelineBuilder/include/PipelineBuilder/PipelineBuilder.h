#pragma once
#ifndef PIPELINE_BUILDER
#define PIPELINE_BUILDER
#include "Pipeline/Pipeline.h"
#include "RenderpassManager/RenderpassManager.h"
#include "CanellaUtility/CanellaUtility.h"
#include <json.hpp>
#include <unordered_map>
#include <fstream>
#include <string>
#include <memory>
namespace Canella
{
	namespace RenderSystem
	{
		namespace VulkanBackend
		{
			using DescriptorSetLayouts = std::unordered_map<std::string, std::shared_ptr<DescriptorSetLayout>>;
			using PipelineLayouts = std::unordered_map<std::string, std::shared_ptr<PipelineLayout>>;
			using Pipelines = std::unordered_map<std::string, std::unique_ptr<Pipeline>>;

			class PipelineBuilder
			{
			public:
				/**
				 * @brief cache the pipelines described in config file
				 * @param pipelineData json metadata containing information about the pipelines
				 * @param descriptor_set_layouts map structure for caching DescripotrsetLayouts
				 * @param pipeline_layouts map structure for caching PipelineLayouts
				 * @param pipelines map structure for caching Pipelines
				 */
				static void cachePipelineData(
					Device* device,
					nlohmann::json& pipelineData,
					DescriptorSetLayouts& descriptor_set_layouts,
					PipelineLayouts& pipeline_layouts,
					Renderpasses& renderpasses,
					Pipelines& pipelines)
				{
					assert(device != nullptr);
					createPipelineLayouts(device, descriptor_set_layouts, pipeline_layouts, pipelineData);
					createPipelines(device,pipelineData,pipelines,pipeline_layouts,renderpasses);
				}

				static std::shared_ptr<DescriptorSetLayout> createDescriptorSetLayout(
					Device* device,
					nlohmann::json resources)
				{
					uint32_t resourceCount = resources["ResourceCount"].get<std::uint32_t>();
					std::vector<ShaderBindingResource> bindingResources(resourceCount);
					for (uint32_t i = 0; i < resourceCount; i++)
					{
						nlohmann::json& bindings = resources["BindingResources"][i];
						ShaderBindingResource bindingResource{};
						bindingResource.type = convert_from_string_shader_resource_type(bindings["Type"].get<std::string>().c_str());
						bindingResource.stages = VK_SHADER_STAGE_MESH_BIT_EXT;
						bindingResource.size = sizeof(ViewProjection);
						bindingResource.binding = bindings["Binding"].get<std::uint32_t>();
						bindingResources[i] = bindingResource;
					}
					auto descriptor = std::make_shared<DescriptorSetLayout>(device, std::move(bindingResources));
					return descriptor;
				}
			private:
				static void createPipelines(
				Device* device,
			    nlohmann::json pipelineData,
			    Pipelines& pipelines,
			    PipelineLayouts& pipelineLayouts,
			    Renderpasses& renderpasses
			    )
				{
					for(auto i = 0 ;i < pipelineData["PipelineData"]["PipelinesCount"].get<int32_t>();++i)
					{
						const auto& pipelines_json = pipelineData["PipelineData"]["Pipelines"][i];
						auto renderpass_key = pipelines_json["Renderpass"].get<std::string>();
						auto pipeline_key = pipelines_json["Key"].get<std::string>();
						auto shader_count = pipelines_json["ShaderCount"].get<std::int32_t>();
						std::vector<Shader> shaders;
						for (auto i = 0 ; i < shader_count; ++i)
						{
							const auto shader_json = pipelines_json["Shaders"][i];
							auto type_string = shader_json["Type"].get<std::string>();
							SHADER_TYPE type = Shader::convert_from_string_shader_type(type_string.c_str());
							auto code = readFile(shader_json["Source"].get<std::string>());
							shaders.push_back(Shader(device,type,code));
						}
					
						PipelineProperties pipelineInfo= {};
						std::vector<std::vector<ATRIBUTES> >atributes;
						pipelineInfo.atributes = atributes;
						pipelineInfo.colorAttachmentsCount = 1;
						pipelineInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
						pipelineInfo.dephTest = 1;
						pipelineInfo.depthBias = 0;
						pipelineInfo.renderpass = &renderpasses[renderpass_key]->get_vk_render_pass();
						pipelineInfo.frontFaceClock = VK_FRONT_FACE_COUNTER_CLOCKWISE;
						pipelineInfo.vertexOffsets = { 0 };
						pipelineInfo.subpass = 0;
						pipelines[pipeline_key] = std::make_unique<Pipeline>(device,*pipelineLayouts[pipeline_key].get(),shaders,pipelineInfo);
					}
				}
				
				
				static void cacheDescriptorSetLayouts(
					Device* device,
					nlohmann::json pipelineData,
					std::vector<VkPushConstantRange>& pushConstants,
					DescriptorSetLayouts& cachedDescriptorLayouts,
					PipelineLayouts& cachedPipelineLayouts)
				{
					const auto descriptorSetLayoutCount = pipelineData["PipelineData"]["DescriptorSetLayoutCounts"].get<std::int32_t>();
					std::vector<std::shared_ptr<DescriptorSetLayout>> descriptor_set_layouts(descriptorSetLayoutCount);

					for (auto i = 0; i < descriptorSetLayoutCount; i++)
						cacheDescriptorSetLayout(device, pipelineData, descriptor_set_layouts, cachedDescriptorLayouts, i);
				}

				static void cacheDescriptorSetLayout(
					Device* device,
					nlohmann::json pipelineData,
					std::vector<std::shared_ptr<DescriptorSetLayout>>& descriptor_set_layouts,
					DescriptorSetLayouts& cachedDescriptorSetLayouts,
					const int i)
				{
					auto key = pipelineData["PipelineData"]["DescriptorSetLayouts"][i]["Key"].get<std::string>();
					descriptor_set_layouts[i] = PipelineBuilder::createDescriptorSetLayout(device,
						pipelineData["PipelineData"]["DescriptorSetLayouts"][i]);
					if (cachedDescriptorSetLayouts.find(key) == cachedDescriptorSetLayouts.end())
						cachedDescriptorSetLayouts[key] = std::move(descriptor_set_layouts[i]);
				}

				static std::vector<char> readFile(const std::string& filename)
				{
					std::ifstream file(filename, std::ios::ate | std::ios::binary);
					if (!file.is_open())
						throw std::runtime_error("failed to open file!");
					size_t fileSize = (size_t)file.tellg();
					std::vector<char> buffer(fileSize);
					file.seekg(0);
					file.read(buffer.data(), fileSize);
					file.close();
					return buffer;
				}

				static void createPipelineLayouts(
					Device* device,
					DescriptorSetLayouts& cachedDescriptorSetLayouts,
					PipelineLayouts& cachedPipelineLayouts,
					nlohmann::json& pipeline_data)
				{
					// Cache DescriptorSetLayouts
					std::vector<VkPushConstantRange> pushConstants;
					cacheDescriptorSetLayouts(device, pipeline_data, pushConstants, cachedDescriptorSetLayouts, cachedPipelineLayouts);
					nlohmann::json pipelines_json = pipeline_data["PipelineData"]["Pipelines"];
					const auto number_of_pipelines = pipeline_data["PipelineData"]["PipelinesCount"].get<std::int32_t>();

					// Cache Pipeline Layouts
					for (auto i = 0; i < number_of_pipelines; i++)
					{
						nlohmann::json pipeline = pipelines_json[i];
						auto key = pipeline["Key"].get<std::string>();
						auto number_of_descriptorset_layouts = pipeline["NumerOfDescriptorSets"].get<std::int32_t>();
						std::vector<std::shared_ptr<DescriptorSetLayout>> descriptor_set_layouts(number_of_descriptorset_layouts);
						for (auto j = 0; j < number_of_descriptorset_layouts; j++)
							descriptor_set_layouts[i] = 
								cachedDescriptorSetLayouts[pipeline["PipelineLayoutDescriptors"][j].get<std::string>()];
						cachedPipelineLayouts[key] = std::make_shared<PipelineLayout>(device,
							std::move(descriptor_set_layouts), pushConstants);
					}
				}
			};
		}
	}
}

#endif