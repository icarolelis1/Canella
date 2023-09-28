#include "Pipeline/Pipeline.h"
#include "CanellaUtility/CanellaUtility.h"
using namespace Canella::RenderSystem::VulkanBackend;

Pipeline::Pipeline(Device* _device,
                   PipelineLayout* _pipelienLayout,
                   std::vector<Shader> shaders,
                   PipelineProperties& info,
                   uint32_t bindingCount)
    : device(_device),
      pipelineLayout((_pipelienLayout))
{
    VertexLayout vertex_layout(info.atributes, info.vertexOffsets, info.vertexBindingCount);

    // Pipeline
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state = initializers::pipelineInputAssemblyStateCreateInfo(info.topology, 0, VK_FALSE);
    VkPipelineRasterizationStateCreateInfo rasterization_state = initializers::pipelineRasterizationStateCreateInfo(info.polygonMode, info.cullMode, info.frontFaceClock);

    VkPipelineColorBlendAttachmentState blend_attachment_state[3];

    blend_attachment_state[0] =  initializers::pipelineColorBlendAttachmentState(
            0xf,
            VK_FALSE);
    blend_attachment_state[1] = blend_attachment_state[0] ;
    blend_attachment_state[2] = blend_attachment_state[0] ;

            VkPipelineColorBlendStateCreateInfo color_blend_state =
            initializers::pipelineColorBlendStateCreateInfo(info.colorAttachmentsCount, blend_attachment_state);

    VkPipelineDepthStencilStateCreateInfo depthStencilState = initializers::pipelineDepthStencilStateCreateInfo(info.dephTest, VK_TRUE, VK_COMPARE_OP_LESS);

    VkPipelineViewportStateCreateInfo viewportState = initializers::pipelineViewportStateCreateInfo(1, 1, VK_DYNAMIC_STATE_VIEWPORT);
    VkPipelineMultisampleStateCreateInfo multisample_state;
    multisample_state = initializers::pipelineMultisampleStateCreateInfo(info.samples);
    std::vector<VkDynamicState> dynamic_state_enables;
    dynamic_state_enables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
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
    pipelineCI.layout = pipelineLayout->get_handle();
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
    if (VkResult r = vkCreateGraphicsPipelines(device->getLogicalDevice(),
                                               VK_NULL_HANDLE,
                                               1,
                                               &pipelineCI,
                                               device->getAllocator(),
                                               &vk_pipeline); r == VK_SUCCESS)
    for (auto& shader : shaders)
        shader.destroyModule();
}
Canella::RenderSystem::VulkanBackend::Pipeline::Pipeline(Device *_device,
                                                         PipelineLayout* _pipeline_layout,
                                                         Shader compute_shader)
        :device(_device),pipelineLayout(_pipeline_layout)
{
    VkComputePipelineCreateInfo  create_info{create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    create_info.stage = compute_shader.getShaderStageInfo();
    create_info.layout = pipelineLayout->get_handle();
    VK_CHECK(vkCreateComputePipelines(_device->getLogicalDevice(),
                                      nullptr,
                                      1,
                                      &create_info,
                                      device->getAllocator(),
                                      &vk_pipeline),"failed to create compute Pipeline");
    compute_shader.destroyModule();

}

VkPipeline& Pipeline::get_pipeline_handle()
{
    return vk_pipeline;
}

PipelineLayout Pipeline::get_pipeline_layout()
{
    return *pipelineLayout;
}

void Pipeline::destroy()
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

    VK_CHECK(vkCreateShaderModule(device->getLogicalDevice(), &createInfo, device->getAllocator(),
                                           &vk_shaderModule),"Failed to crete shader module");

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


DescriptorSetLayout::DescriptorSetLayout(Device* device,
                                         const std::vector<ShaderBindingResource> _resources,
                                         bool push_descriptor,
                                         const char* description)
{
    foo(_resources);

    VkDescriptorSetLayoutCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    create_info.pBindings = bindings.data();
    if(push_descriptor)  create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;

    VkResult result = vkCreateDescriptorSetLayout(device->getLogicalDevice(),
                                                  &create_info,
                                                  device->getAllocator(),
                                                  &vk_descriptorSetLayout);

    if (result != VK_SUCCESS)
        Canella::Logger::Error("Failed to create DescriptorSetLayout\n");
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
        case ShaderResourceType::IMAGE_SAMPLER:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case ShaderResourceType::INPUT_ATTACHMENT:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        case ShaderResourceType::STORAGE_BUFFER:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case ShaderResourceType::IMAGE_STORAGE:
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case ShaderResourceType::IMAGE_SAMPLER_CUBE:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        default:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
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
        pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<uint32_t>(_pushConstants.size());
        pipelineLayoutCreateInfo.pPushConstantRanges = _pushConstants.data();
    }

    if (const VkResult result = vkCreatePipelineLayout(device->getLogicalDevice(), &pipelineLayoutCreateInfo,
                                                       device->getAllocator(), &vk_pipelineLayout); result != VK_SUCCESS)
        Logger::Error("Failed to create pipelineLayout layout\n");
    else
        Logger::Debug("Successfully Created PipelineLayout");
}

VkPipelineLayout PipelineLayout::get_handle()
{
    return vk_pipelineLayout;
}

void PipelineLayout::destroy(Device* device)
{
    vkDestroyPipelineLayout(device->getLogicalDevice(), vk_pipelineLayout, device->getAllocator());
}
