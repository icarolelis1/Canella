#include "Pipeline/Pipeline.h"
#include "CanellaUtility/CanellaUtility.h"

using namespace Canella::RenderSystem::VulkanBackend;

Pipeline::Pipeline(Device *_device,
				   PipelineLayout _pipelienLayout,
				   std::unique_ptr<Shader> _vertexShader,
				   std::unique_ptr<Shader> _fragmentShader,
				   PipelineProperties &info,
				   uint32_t bindingCount)
	: vertexShader(std::move(_vertexShader)), fragmentShader(std::move(_fragmentShader)), device(_device), pipelineLayout(std::move(_pipelienLayout))
{
	VertexLayout vertexLayout(info.atributes, info.vertexOffsets, info.vertexBindingCount);

	// Pipeline
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = initializers::pipelineInputAssemblyStateCreateInfo(info.topology, 0, VK_FALSE);
	VkPipelineRasterizationStateCreateInfo rasterizationState = initializers::pipelineRasterizationStateCreateInfo(info.polygonMode, info.cullMode, info.frontFaceClock);

	if (info.depthBias)
	{

		rasterizationState.depthBiasEnable = 1;
		rasterizationState.depthBiasConstantFactor = 4.0f;
		rasterizationState.depthBiasSlopeFactor = 1.50f;
	}

	VkPipelineColorBlendAttachmentState *colorBlends = new VkPipelineColorBlendAttachmentState[info.colorAttachmentsCount];
	VkPipelineColorBlendAttachmentState blendAttachmentState;

	for (unsigned int i = 0; i < info.colorAttachmentsCount; i++)
	{

		if (!info.alphablending)
		{
			blendAttachmentState = initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
		}

		else
		{
			// Premulitplied alpha
			blendAttachmentState.blendEnable = VK_TRUE;
			blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
			blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
			blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		}
		colorBlends[i] = blendAttachmentState;
	}

	VkPipelineColorBlendStateCreateInfo colorBlendState = initializers::pipelineColorBlendStateCreateInfo(info.colorAttachmentsCount, colorBlends);
	VkPipelineDepthStencilStateCreateInfo depthStencilState = initializers::pipelineDepthStencilStateCreateInfo(info.dephTest, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

	if (info.alphablending)
	{

		depthStencilState.depthWriteEnable = VK_FALSE;
	}

	VkPipelineViewportStateCreateInfo viewportState = initializers::pipelineViewportStateCreateInfo(1, 1, VK_DYNAMIC_STATE_VIEWPORT);
	VkPipelineMultisampleStateCreateInfo multisampleState = initializers::pipelineMultisampleStateCreateInfo(info.samples);
	std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

	// Vertex input state
	std::vector<VkVertexInputBindingDescription> bds;

	VkPipelineVertexInputStateCreateInfo vertexInputState{};
	std::vector<VkVertexInputAttributeDescription> atribDescription;

	for (unsigned int i = 0; i < info.vertexBindingCount; i++)
	{

		VkVertexInputBindingDescription bindingDescription = vertexLayout.getBinding(static_cast<uint32_t>(i));
		bds.push_back(bindingDescription);
		vertexLayout.getAttributeDescription(static_cast<uint32_t>(i), atribDescription);
	}
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(atribDescription.size());
	vertexInputState.pVertexAttributeDescriptions = atribDescription.data();

	if (info.vertexBindingCount != 0)
	{
		vertexInputState.pVertexBindingDescriptions = bds.data();
		vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bds.size());
	}

	else
		vertexInputState.vertexBindingDescriptionCount = 0;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

	shaderStages.resize(2);

	shaderStages = {vertexShader->getShaderStageInfo(), fragmentShader->getShaderStageInfo()};

	VkGraphicsPipelineCreateInfo pipelineCI{};
	pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCI.layout = pipelineLayout.getHandle();
	pipelineCI.renderPass = *info.renderpass;
	pipelineCI.flags = 0;
	pipelineCI.basePipelineIndex = -1;
	pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCI.pInputAssemblyState = &inputAssemblyState;
	pipelineCI.pRasterizationState = &rasterizationState;
	pipelineCI.pColorBlendState = &colorBlendState;
	pipelineCI.pMultisampleState = &multisampleState;
	pipelineCI.pViewportState = &viewportState;
	pipelineCI.pDepthStencilState = &depthStencilState;
	pipelineCI.pDynamicState = &dynamicState;
	pipelineCI.stageCount = 2;
	pipelineCI.pStages = shaderStages.data();
	pipelineCI.pVertexInputState = &vertexInputState;
	pipelineCI.subpass = info.subpass;
	VkResult r = vkCreateGraphicsPipelines(device->getLogicalDevice(), vk_cache, 1, &pipelineCI, device->getAllocator(), &vk_pipeline);

	if (r == VK_SUCCESS)
		std::cout << "Successfully created pipeline\n";

	vertexShader->destroyModule();
	fragmentShader->destroyModule();

	delete[] colorBlends;
}

VkPipeline &Pipeline::getPipelineHandle()
{
	return vk_pipeline;
}

PipelineLayout Pipeline::getPipelineLayoutHandle()
{
	return pipelineLayout;
}

Pipeline::~Pipeline()
{
	vkDestroyPipeline(device->getLogicalDevice(), vk_pipeline, device->getAllocator());
}

Shader::Shader(Device *_device, SHADER_TYPE _type, const std::vector<char> _code) : device(_device), type(_type), code(_code)
{
}

VkPipelineShaderStageCreateInfo Shader::getShaderStageInfo()
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

	VkResult result = vkCreateShaderModule(device->getLogicalDevice(), &createInfo, device->getAllocator(), &vk_shaderModule);

	VkPipelineShaderStageCreateInfo stageCreateInfo = {};
	stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	if (type == SHADER_TYPE::VERTEX_SHADER)
		stageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	else if (type == SHADER_TYPE::FRAGMENT_SHADER)
		stageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	else if (type == SHADER_TYPE::COMPUTE_SHADER)
		stageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	else if (type == SHADER_TYPE::MESH_SHADER)
		stageCreateInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
	else if (type == SHADER_TYPE::TASK_SHADER)
		stageCreateInfo.stage = VK_SHADER_STAGE_TASK_BIT_EXT;

	stageCreateInfo.module = vk_shaderModule;
	stageCreateInfo.pName = "main";

	return stageCreateInfo;
}

void Shader::destroyModule()
{
	vkDestroyShaderModule(device->getLogicalDevice(), vk_shaderModule, nullptr);
}

void DescriptorSetLayout::build(Device *device, const std::vector<ShaderBindingResource> _resources, const char *description)
{

	for (auto &resource : _resources)
	{
		auto type = DescriptorSetLayout::getDescriptorType(resource.type);
		VkDescriptorSetLayoutBinding layout_binding{};
		layout_binding.binding = resource.binding;
		layout_binding.descriptorCount = 1;
		layout_binding.descriptorType = type;
		layout_binding.stageFlags = static_cast<VkShaderStageFlags>(resource.stages);
		bindings.push_back(layout_binding);
	}

	VkDescriptorSetLayoutCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	create_info.bindingCount = static_cast<uint32_t>(bindings.size());
	create_info.pBindings = bindings.data();

	VkResult result = vkCreateDescriptorSetLayout(device->getLogicalDevice(), &create_info, device->getAllocator(), &vk_descriptorSetLayout);

	if (result != VK_SUCCESS)
		Canella::Logger::Error("Failed to create DescriptorSetLayout\n");
	if(result == VK_SUCCESS)
		Canella::Logger::Error("Successfully Created DescriptorSetLayout");
}

void DescriptorSetLayout::destroy(Device *device)
{
	vkDestroyDescriptorSetLayout(device->getLogicalDevice(), vk_descriptorSetLayout, device->getAllocator());
}

std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayout::getBindings() const
{
	return (bindings);
}

VkDescriptorSetLayout &DescriptorSetLayout::getDescriptorLayoutHandle()
{
	return vk_descriptorSetLayout;
}

VkDescriptorType DescriptorSetLayout::getDescriptorType(ShaderResourceType type)
{
	switch (type)
	{

	case ShaderResourceType::UNIFORM_BUFFER:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		break;

	case ShaderResourceType::IMAGE_SAMPLER:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		break;

	case ShaderResourceType::INPUT_ATTACHMENT:
		return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		break;

	case ShaderResourceType::UNIFORM_DYNAMIC:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		break;
	}
}

void PipelineLayout::build(Device *_device, std::vector<DescriptorSetLayout> _descriptors, std::vector<VkPushConstantRange> _pushConstants)
{

	std::vector<VkDescriptorSetLayout> handles;

	for (auto &descriptor : _descriptors)
	{
		handles.push_back(descriptor.getDescriptorLayoutHandle());
	}

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(handles.size());

	if (handles.size() > 0)
		pipelineLayoutCreateInfo.pSetLayouts = handles.data();

	if (_pushConstants.size() > 0)
	{
		pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
		pipelineLayoutCreateInfo.pPushConstantRanges = pushConstants.data();
	}
	VkResult result = vkCreatePipelineLayout(_device->getLogicalDevice(), &pipelineLayoutCreateInfo, device->getAllocator(), &vk_pipelineLayout);

	if (result != VK_SUCCESS)
		Canella::Logger::Error("Failed to create descriptorset layout\n");
}

VkPipelineLayout PipelineLayout::getHandle()
{
	return vk_pipelineLayout;
}

void PipelineLayout::destroy(Device * device)
{
	vkDestroyPipelineLayout(device->getLogicalDevice(), vk_pipelineLayout, device->getAllocator());
}

ComputePipeline::ComputePipeline(Device *_device, PipelineLayout _pipelienLayout, std::unique_ptr<Shader> _computeShader)
	: device(_device), pipelineLayout(std::move(pipelineLayout)), computeShader(std::move(_computeShader))
{
	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.layout = pipelineLayout.getHandle();
	computePipelineCreateInfo.flags = 0;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages.resize(1);

	shaderStages = {computeShader->getShaderStageInfo()};

	computePipelineCreateInfo.stage = shaderStages[0];

	if ((vkCreateComputePipelines(device->getLogicalDevice(), vk_cache, 1, &computePipelineCreateInfo, device->getAllocator(), &vk_pipeline) != VK_SUCCESS))
	{
		std::cout << "Failed to create compute pipeline\n\n";
	}
}

ComputePipeline::~ComputePipeline()
{
	vkDestroyPipeline(device->getLogicalDevice(), vk_pipeline, device->getAllocator());
}