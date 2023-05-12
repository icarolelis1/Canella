#include "Pipeline/Pipeline.h"
#include "CanellaUtility/CanellaUtility.h"

using namespace Canella::RenderSystem::VulkanBackend;

Pipeline::Pipeline(Device* _device,
                   PipelineLayout _pipelienLayout,
                   std::vector<Shader> shaders,
                   PipelineProperties& info,
                   uint32_t bindingCount)
    : device(_device),
      pipelineLayout(std::move(_pipelienLayout))
{
    VertexLayout vertex_layout(info.atributes, info.vertexOffsets, info.vertexBindingCount);

    // Pipeline
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = initializers::pipelineInputAssemblyStateCreateInfo(
        info.topology, 0, VK_FALSE);
    VkPipelineRasterizationStateCreateInfo rasterization_state = initializers::pipelineRasterizationStateCreateInfo(
        info.polygonMode, info.cullMode, info.frontFaceClock);

    if (info.depthBias)
    {
        rasterization_state.depthBiasEnable = 1;
        rasterization_state.depthBiasConstantFactor = 4.0f;
        rasterization_state.depthBiasSlopeFactor = 1.50f;
    }

    auto* color_blends = new VkPipelineColorBlendAttachmentState[info.
        colorAttachmentsCount];
    VkPipelineColorBlendAttachmentState blend_attachment_state;

    for (unsigned int i = 0; i < info.colorAttachmentsCount; i++)
    {
        if (!info.alphablending)
            blend_attachment_state = initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
        else
        {
            // Premulitplied alpha
            blend_attachment_state.blendEnable = VK_TRUE;
            blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
            blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
            blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }
        color_blends[i] = blend_attachment_state;
    }

    VkPipelineColorBlendStateCreateInfo color_blend_state;
    color_blend_state = initializers::pipelineColorBlendStateCreateInfo(
        info.colorAttachmentsCount, color_blends);
    VkPipelineDepthStencilStateCreateInfo depthStencilState = initializers::pipelineDepthStencilStateCreateInfo(
        info.dephTest, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

    if (info.alphablending)
    {
        depthStencilState.depthWriteEnable = VK_FALSE;
    }

    VkPipelineViewportStateCreateInfo viewportState = initializers::pipelineViewportStateCreateInfo(
        1, 1, VK_DYNAMIC_STATE_VIEWPORT);
    VkPipelineMultisampleStateCreateInfo multisample_state;
    multisample_state = initializers::pipelineMultisampleStateCreateInfo(info.samples);
    std::vector<VkDynamicState> dynamic_state_enables;
    dynamic_state_enables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state;
    dynamic_state = initializers::pipelineDynamicStateCreateInfo(dynamic_state_enables);

    // Vertex input state
    std::vector<VkVertexInputBindingDescription> bds;

    VkPipelineVertexInputStateCreateInfo vertex_input_state{};
    std::vector<VkVertexInputAttributeDescription> atrib_description;

    for (unsigned int i = 0; i < info.vertexBindingCount; i++)
    {
        VkVertexInputBindingDescription bindingDescription = vertex_layout.getBinding(static_cast<uint32_t>(i));
        bds.push_back(bindingDescription);
        vertex_layout.getAttributeDescription(static_cast<uint32_t>(i), atrib_description);
    }
    vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_state.vertexAttributeDescriptionCount = static_cast<uint32_t>(atrib_description.size());
    vertex_input_state.pVertexAttributeDescriptions = atrib_description.data();

    if (info.vertexBindingCount != 0)
    {
        vertex_input_state.pVertexBindingDescriptions = bds.data();
        vertex_input_state.vertexBindingDescriptionCount = static_cast<uint32_t>(bds.size());
    }
    else
        vertex_input_state.vertexBindingDescriptionCount = 0;

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    shader_stages.reserve(shaders.size());
    for (auto& shader : shaders)
        shader_stages.push_back(shader.getShaderStageInfo());

    VkGraphicsPipelineCreateInfo pipelineCI{};
    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.layout = pipelineLayout.getHandle();
    pipelineCI.renderPass = *info.renderpass;
    pipelineCI.flags = 0;
    pipelineCI.basePipelineIndex = -1;
    pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCI.pInputAssemblyState = &input_assembly_state;
    pipelineCI.pRasterizationState = &rasterization_state;
    pipelineCI.pColorBlendState = &color_blend_state;
    pipelineCI.pMultisampleState = &multisample_state;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pDynamicState = &dynamic_state;
    pipelineCI.stageCount = static_cast<int32_t>(shaders.size());
    pipelineCI.pStages = shader_stages.data();
    if (!vertex_layout.atributes.empty())
        pipelineCI.pVertexInputState = &vertex_input_state;
    else
        pipelineCI.pVertexInputState = nullptr;
    pipelineCI.subpass = info.subpass;
    
    /*
        TODO cache pipeline 
        ref https://medium.com/@zeuxcg/creating-a-robust-pipeline-cache-with-vulkan-961d09416cda
    */
    if (VkResult r = vkCreateGraphicsPipelines(device->getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineCI,
                                               device->getAllocator(),
                                               &vk_pipeline); r == VK_SUCCESS)
        Logger::Debug("Successfully created pipeline\n");
    for (auto& shader : shaders)
        shader.destroyModule();

    delete[] color_blends;
}

VkPipeline& Pipeline::getPipelineHandle()
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

Shader::Shader(Device* _device, SHADER_TYPE _type, const std::vector<char> _code) : device(_device), type(_type),
    code(_code)
{
}

VkPipelineShaderStageCreateInfo Shader::getShaderStageInfo()
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkResult result = vkCreateShaderModule(device->getLogicalDevice(), &createInfo, device->getAllocator(),
                                           &vk_shaderModule);

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

/**
 * \brief Wrapper for a VkDescriptorSetLayout
 * \param device Vulkan device
 * \param _resources *shaderBidings resources
 * \param description description of descriptorset layout
 */
DescriptorSetLayout::DescriptorSetLayout(Device* device, const std::vector<ShaderBindingResource> _resources,
                                         const char* description)
{
    foo(_resources);

    VkDescriptorSetLayoutCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    create_info.pBindings = bindings.data();

    VkResult result = vkCreateDescriptorSetLayout(device->getLogicalDevice(), &create_info, device->getAllocator(),
                                                  &vk_descriptorSetLayout);

    if (result != VK_SUCCESS)
        Canella::Logger::Error("Failed to create DescriptorSetLayout\n");
    if (result == VK_SUCCESS)
        Canella::Logger::Error("Successfully Created DescriptorSetLayout");
}

void DescriptorSetLayout::foo(const std::vector<ShaderBindingResource> &_resources) {
    for (auto& resource : _resources)
    {
        auto type = getDescriptorType(resource.type);
        VkDescriptorSetLayoutBinding layout_binding{};
        layout_binding.binding = resource.binding;
        layout_binding.descriptorCount = 1;
        layout_binding.descriptorType = type;
        layout_binding.stageFlags = static_cast<VkShaderStageFlags>(resource.stages);
        bindings.push_back(layout_binding);
    }
}

void DescriptorSetLayout::destroy(Device* device)
{
    vkDestroyDescriptorSetLayout(device->getLogicalDevice(), vk_descriptorSetLayout, device->getAllocator());
}

std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayout::getBindings() const
{
    return (bindings);
}

VkDescriptorSetLayout& DescriptorSetLayout::getDescriptorLayoutHandle()
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
    case ShaderResourceType::STORAGE_BUFFER:
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        break;
    default:
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        break;
    }
}

PipelineLayout::PipelineLayout(
    Device* device,
    std::vector<std::shared_ptr<DescriptorSetLayout>> _descriptors,
    std::vector<VkPushConstantRange> _pushConstants)
{
    std::vector<VkDescriptorSetLayout> handles;
    for (auto& descriptor : _descriptors)
        handles.push_back(descriptor.get()->getDescriptorLayoutHandle());

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(handles.size());

    if (!handles.empty())
        pipelineLayoutCreateInfo.pSetLayouts = handles.data();

    if (!_pushConstants.empty())
    {
        pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        pipelineLayoutCreateInfo.pPushConstantRanges = pushConstants.data();
    }

    if (const VkResult result = vkCreatePipelineLayout(device->getLogicalDevice(), &pipelineLayoutCreateInfo,
                                                       device->getAllocator(), &vk_pipelineLayout); result != VK_SUCCESS)
        Logger::Error("Failed to create pipelineLayout layout\n");
    else
        Logger::Debug("Successfully Created PipelineLayout");
}

VkPipelineLayout PipelineLayout::getHandle()
{
    return vk_pipelineLayout;
}

void PipelineLayout::destroy(Device* device)
{
    vkDestroyPipelineLayout(device->getLogicalDevice(), vk_pipelineLayout, device->getAllocator());
}

ComputePipeline::ComputePipeline(Device* _device, PipelineLayout _pipelienLayout,
                                 std::unique_ptr<Shader> _computeShader)
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

    if ((vkCreateComputePipelines(device->getLogicalDevice(), vk_cache, 1, &computePipelineCreateInfo,
                                  device->getAllocator(), &vk_pipeline) != VK_SUCCESS))
    {
        std::cout << "Failed to create compute pipeline\n\n";
    }
}

ComputePipeline::~ComputePipeline()
{
    vkDestroyPipeline(device->getLogicalDevice(), vk_pipeline, device->getAllocator());
}
