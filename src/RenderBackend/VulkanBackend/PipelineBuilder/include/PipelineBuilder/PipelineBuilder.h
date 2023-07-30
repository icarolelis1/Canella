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
				static void cache_pipeline_data(
					Device *device,
					nlohmann::json &pipelineData,
					DescriptorSetLayouts &descriptor_set_layouts,
					PipelineLayouts &pipeline_layouts,
					Renderpasses &renderpasses,
					Pipelines &pipelines)
				{
					assert(device != nullptr);
					createPipelineLayouts(device, descriptor_set_layouts, pipeline_layouts, pipelineData);
					createPipelines(device, pipelineData, pipelines, pipeline_layouts, renderpasses);
				}

				static std::shared_ptr<DescriptorSetLayout> createDescriptorSetLayout(
					Device *device,
					nlohmann::json resources)
				{
					uint32_t resourceCount = resources["ResourceCount"].get<std::uint32_t>();
					std::vector<ShaderBindingResource> bindingResources(resourceCount);
					for (uint32_t i = 0; i < resourceCount; i++)
					{
						nlohmann::json &bindings = resources["BindingResources"][i];
						ShaderBindingResource bindingResource{};
						bindingResource.type = convert_from_string_shader_resource_type(bindings["Type"].get<std::string>().c_str());
						bindingResource.stages = read_shader_stage_from_json(bindings["Stage"]);
						// bindingResource.size = get_size_of_structure(bindings["Key"].get<std::string>().c_str());
						bindingResource.binding = bindings["Binding"].get<std::uint32_t>();
						bindingResources[i] = bindingResource;
					}
                    auto is_push_descriptor = resources["PushDescriptor"].get<bool>();
					auto descriptor = std::make_shared<DescriptorSetLayout>(device,
                                                                            std::move(bindingResources),
                                                                            is_push_descriptor);
					return descriptor;
				}

			private:
				static void createPipelines(
					Device *device,
					nlohmann::json pipelineData,
					Pipelines &pipelines,
					PipelineLayouts &pipelineLayouts,
					Renderpasses &renderpasses)
				{
                    auto pipeline_count = pipelineData["PipelineData"]["PipelinesCount"].get<int32_t>();
					for (auto i = 0; i <pipeline_count ; ++i)
					{
						const auto &pipelines_json = pipelineData["PipelineData"]["Pipelines"][i];

						if (pipelines_json["IsCompute"].get<bool>())
						{
							create_compute_pipeline(device, pipelines_json, pipelines, pipelineLayouts);
                            continue;
						}
						auto renderpass_key = pipelines_json["Renderpass"].get<std::string>();
						auto pipeline_key = pipelines_json["Key"].get<std::string>();
						auto shader_count = pipelines_json["ShaderCount"].get<std::int32_t>();
						std::vector<Shader> shaders;
						for (auto x = 0; x < shader_count; ++x)
						{
							const auto shader_json = pipelines_json["Shaders"][x];
							auto type_string = shader_json["Type"].get<std::string>();
							SHADER_TYPE type = Shader::convert_from_string_shader_type(type_string.c_str());
							auto code = readFile(shader_json["Source"].get<std::string>());
							shaders.push_back(Shader(device, type, code));
						}

						PipelineProperties pipelineInfo = {};
						std::vector<std::vector<ATRIBUTES>> atributes;
						pipelineInfo.atributes = atributes;
						pipelineInfo.colorAttachmentsCount = 1;
						pipelineInfo.cullMode = VK_CULL_MODE_BACK_BIT;
						pipelineInfo.dephTest = VK_TRUE;
						pipelineInfo.depthBias = 0;
						pipelineInfo.renderpass = &renderpasses[renderpass_key]->get_vk_render_pass();
						pipelineInfo.frontFaceClock = VK_FRONT_FACE_CLOCKWISE;
						pipelineInfo.vertexOffsets = {0};
						pipelineInfo.subpass = 0;
						pipelines[pipeline_key] = std::make_unique<Pipeline>(device,
                                                                             pipelineLayouts[pipeline_key].get(), shaders, pipelineInfo);
					}
				}

				static void cacheDescriptorSetLayouts(
					Device *device,
					nlohmann::json pipelineData,
					DescriptorSetLayouts &cachedDescriptorLayouts,
					PipelineLayouts &cachedPipelineLayouts)
				{
					const auto descriptorSetLayoutCount = pipelineData["PipelineData"]["DescriptorSetLayoutCounts"].get<std::int32_t>();
					std::vector<std::shared_ptr<DescriptorSetLayout>> descriptor_set_layouts(descriptorSetLayoutCount);

					for (auto i = 0; i < descriptorSetLayoutCount; i++)
						cacheDescriptorSetLayout(device, pipelineData, descriptor_set_layouts, cachedDescriptorLayouts, i);
				}

				static void cacheDescriptorSetLayout(
					Device *device,
					nlohmann::json pipelineData,
					std::vector<std::shared_ptr<DescriptorSetLayout>> &descriptor_set_layouts,
					DescriptorSetLayouts &cachedDescriptorSetLayouts,
					const int i)
				{
					auto key = pipelineData["PipelineData"]["DescriptorSetLayouts"][i]["Key"].get<std::string>();
					descriptor_set_layouts[i] = PipelineBuilder::createDescriptorSetLayout(device,
																						   pipelineData["PipelineData"]
																									   ["DescriptorSetLayouts"][i]);
					if (cachedDescriptorSetLayouts.find(key) == cachedDescriptorSetLayouts.end())
						cachedDescriptorSetLayouts[key] = std::move(descriptor_set_layouts[i]);
				}

				static std::vector<char> readFile(const std::string &filename)
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
					Device *device,
					DescriptorSetLayouts &cachedDescriptorSetLayouts,
					PipelineLayouts &cachedPipelineLayouts,
					nlohmann::json &pipeline_data)
				{
					// Cache DescriptorSetLayouts
					cacheDescriptorSetLayouts(device, pipeline_data, cachedDescriptorSetLayouts, cachedPipelineLayouts);
					nlohmann::json pipelines_json = pipeline_data["PipelineData"]["Pipelines"];
					const auto number_of_pipelines = pipeline_data["PipelineData"]["PipelinesCount"].get<std::int32_t>();

					// Cache Pipeline Layouts
					for (auto i = 0; i < number_of_pipelines; i++)
					{
						std::vector<VkPushConstantRange> push_constants;

						nlohmann::json pipeline = pipelines_json[i];
						auto key = pipeline["Key"].get<std::string>();
						auto push_constant_count = pipeline["PushConstantRangesCount"].get<std::uint32_t>();

						push_constants.resize(push_constant_count);
						for (auto &push_constant : push_constants)
						{
							auto &push_constants_data = pipeline["PushConstants"];
							push_constant.size = push_constants_data["Size"].get<uint32_t>();
							push_constant.stageFlags = read_shader_stage_from_json(push_constants_data["Stage"]);
							push_constant.offset = 0;
						}

						auto number_of_descriptorset_layouts = pipeline["NumerOfDescriptorSets"].get<std::int32_t>();
						std::vector<std::shared_ptr<DescriptorSetLayout>> descriptor_set_layouts(number_of_descriptorset_layouts);
						for (auto j = 0; j < number_of_descriptorset_layouts; j++)
							descriptor_set_layouts[j] =
								cachedDescriptorSetLayouts[pipeline["PipelineLayoutDescriptors"][j].get<std::string>()];
						cachedPipelineLayouts[key] = std::make_shared<PipelineLayout>(device,
																					  std::move(descriptor_set_layouts), push_constants);
					}
				}

				static void create_compute_pipeline(Device *device,
													const nlohmann::json &pipelines_json,
													Pipelines &pipelines,
													PipelineLayouts &pipelineLayouts)
				{
					auto pipeline_key = pipelines_json["Key"].get<std::string>();
					const auto shader_json = pipelines_json["Shader"];
					auto type_string = shader_json["Type"].get<std::string>();
					SHADER_TYPE type = Shader::convert_from_string_shader_type(type_string.c_str());
					auto code = readFile(shader_json["Source"].get<std::string>());
					Shader shader(device, type, code);
					pipelines[pipeline_key] = std::make_unique<Pipeline>(device, pipelineLayouts[pipeline_key].get(), shader);
				}
			};
		}
	}
}

#endif