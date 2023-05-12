#ifndef VULKAN_PIPELINE
#define VULKAN_PIPELINE
#include "Device/Device.h"
#include <vector>
#include <memory>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
namespace Canella {
	namespace RenderSystem
	{
		namespace VulkanBackend
		{
			
			struct ViewProjection{
				glm::mat4 projection;
				glm::mat4 model;
				glm::mat4 view;
			};

			enum class ATRIBUTES
			{
				NONE,
				FLOAT,
				VEC4,
				VEC4SI,
				VEC2,
				VEC3
			};

			enum class ShaderResourceType {
				IMAGE_SAMPLER,
				UNIFORM_BUFFER,
				PUSH_CONSTANT,
				STORAGE_BUFFER,
				INPUT_ATTACHMENT,
				IMAGE_STORAGE,
				UNIFORM_DYNAMIC
			};

			struct ShaderBindingResource {
				ShaderResourceType type;
				uint32_t binding;
				uint32_t size;
				VkShaderStageFlags stages;
			};

			struct VertexLayout {

			public:
				std::vector<std::vector<ATRIBUTES>> atributes;
				VertexLayout(std::vector<std::vector<ATRIBUTES> >atribs_, std::vector<uint32_t>  vertexOffsets, uint32_t vertexBindingCount = 1) :numberOfBindings(vertexBindingCount) {
					//todo ??
					atributes.resize(vertexBindingCount);
					atributes = (atribs_);
				}
				uint32_t numberOfBindings;
				uint32_t getVertexStride(uint32_t binding) {

					uint32_t res = 0;

					uint32_t firstOffset = 0;
					for (auto& component : atributes[binding])
					{
						switch (component)
						{
						case ATRIBUTES::VEC2:
							res += 2 * sizeof(float);
							break;
						case ATRIBUTES::VEC4:
							res += 4 * sizeof(float);
							break;
						case ATRIBUTES::VEC3:
							res += 3 * sizeof(float);
							break;
						case ATRIBUTES::VEC4SI:
							res += 4 * sizeof(int);
							break;

						}
					}

					return res;
				}
				VkVertexInputBindingDescription getBinding(int bindingNumber) {

					VkVertexInputBindingDescription binding;
					binding.binding = static_cast<uint32_t>(bindingNumber);
					binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
					binding.stride = getVertexStride(static_cast<uint32_t>(bindingNumber));

					return binding;
				}
				void getAttributeDescription(uint32_t bindingNumber, std::vector<VkVertexInputAttributeDescription>& atributeDescriptions) {

					uint32_t offset = 0;
					size_t i;

					if (bindingNumber > 0)
						i = atributes[bindingNumber - 1].size();

					else {
						i = 0;
					}

					for (auto& attb : atributes[bindingNumber]) {
						VkVertexInputAttributeDescription atribute{};
						atribute.binding = bindingNumber;

						switch (attb) {

						case(ATRIBUTES::VEC2):
							atribute.format = VK_FORMAT_R32G32_SFLOAT;
							atribute.location = static_cast<uint32_t>(i++);
							atribute.offset = offset;
							atributeDescriptions.push_back(atribute);
							offset += sizeof(float) * 2;
							break;

						case(ATRIBUTES::VEC3):
							atribute.format = VK_FORMAT_R32G32B32_SFLOAT;
							atribute.location = static_cast<uint32_t>(i++);
							atribute.offset = offset;
							atributeDescriptions.push_back(atribute);
							offset += sizeof(float) * 3;
							break;

						case(ATRIBUTES::VEC4):
							atribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
							atribute.location = static_cast<uint32_t>(i++);
							atribute.offset = offset;
							atributeDescriptions.push_back(atribute);
							offset += sizeof(float) * 4;
							break;
						case(ATRIBUTES::VEC4SI):
							atribute.format = VK_FORMAT_R32G32B32A32_SINT;
							atribute.location = static_cast<uint32_t>(i++);
							atribute.offset = offset;
							atributeDescriptions.push_back(atribute);
							offset += sizeof(int) * 4;
							break;
						default: break;
						}
					}
					offset = 0;
				}
			};

			struct PipelineProperties {
				VkRenderPass* renderpass;
				uint32_t subpass = 0;
				VkCullModeFlagBits cullMode = VK_CULL_MODE_NONE;
				VkBool32 dephTest = VK_TRUE;
				VkFrontFace frontFaceClock = VK_FRONT_FACE_COUNTER_CLOCKWISE;
				VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
				VkShaderModule vertShaderModule;
				VkShaderModule fragShaderModule;
				VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
				uint32_t vertexBindingCount = 0;
				uint32_t colorAttachmentsCount = 1;
				uint32_t pushConstanteCount = 0;
				const char* fragmentShaderPath;
				const char* vertexShaderPath;
				std::vector<std::vector<ATRIBUTES> >atributes;
				bool depthBias = 0;
				std::vector<uint32_t> vertexOffsets;
				bool alphablending = false;
			};

			enum class SHADER_TYPE {
				FRAGMENT_SHADER,
				VERTEX_SHADER,
				GEOMETRY_SHADER,
				COMPUTE_SHADER,
				MESH_SHADER,
				TASK_SHADER
			};

			class Shader {

			public:
				Shader(Device* _device, SHADER_TYPE _type, const std::vector<char> _code);
				VkPipelineShaderStageCreateInfo getShaderStageInfo();
				void destroyModule();
				static  SHADER_TYPE convert_from_string_shader_type(const char* type)
				{
					{
						if (strcmp( type,"ModelMesh") == 0)
							return SHADER_TYPE::MESH_SHADER;
						if (strcmp(type , "Fragment")==0)
							return SHADER_TYPE::FRAGMENT_SHADER;
						if (strcmp(type,"Task") ==0)
							return SHADER_TYPE::TASK_SHADER;
						return SHADER_TYPE::VERTEX_SHADER;
					}
				}
		
			private:
				const std::vector<char> code;
				Device* device;
				SHADER_TYPE type;
				VkShaderModule vk_shaderModule;
			};

			class DescriptorSetLayout {

			public:
				DescriptorSetLayout(Device* _device, const std::vector<ShaderBindingResource> _resources, const char* description = "GenericSet");
				std::vector< VkDescriptorSetLayoutBinding> getBindings() const;
				VkDescriptorSetLayout& getDescriptorLayoutHandle();
				void destroy(Device*);

			private:
				VkDescriptorSetLayout vk_descriptorSetLayout;
				VkDescriptorType getDescriptorType(ShaderResourceType type);
				std::vector< VkDescriptorSetLayoutBinding> bindings;

                void foo(const std::vector<ShaderBindingResource> &_resources);
            };

			class PipelineLayout {

			public:
				PipelineLayout(Device* _device,
					std::vector<std::shared_ptr<DescriptorSetLayout>> _descriptors,
					std::vector<VkPushConstantRange> _pushConstants);
				VkPipelineLayout getHandle();
				void destroy(Device * device);

			private:
				std::vector<DescriptorSetLayout> descriptors;
				std::vector<VkPushConstantRange> pushConstants;
				VkPipelineLayout vk_pipelineLayout;

			};

			class Pipeline {

			public:
				Pipeline(Device* _device, PipelineLayout _pipelienLayout, std::vector<Shader> shaders, PipelineProperties& info, uint32_t bindingCount =0);
				VkPipeline& getPipelineHandle();
				PipelineLayout getPipelineLayoutHandle();
				const char* id;
				~Pipeline();
			private:	
				Device* device;
				VkPipeline vk_pipeline;
				VkPipelineCache vk_cache;
				PipelineLayout pipelineLayout;
			};


			class ComputePipeline {

			public:
				ComputePipeline(Device* _device, PipelineLayout _pipelienLayout, std::unique_ptr<Shader> _computeShader);
				~ComputePipeline();

			private:
				std::unique_ptr<Shader> computeShader;
				Device* device;
				VkPipeline vk_pipeline;
				VkPipelineCache vk_cache;
				PipelineLayout pipelineLayout;

			};

		}
	}
}
#endif