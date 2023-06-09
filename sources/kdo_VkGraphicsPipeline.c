/* *********************************************************************************** */
/*       :::    ::: ::::::::  :::::::::   ::::::::  :::    :::     :::     ::::::::::: */
/*      :+:   :+: :+:    :+: :+:    :+: :+:    :+: :+:   :+:    :+: :+:       :+:      */
/*     +:+  +:+  +:+    +:+ +:+    +:+ +:+    +:+ +:+  +:+    +:+   +:+      +:+       */
/*    +#++:++   +#+    +:+ +#+    +:+ +#+    +:+ +#++:++    +#++:++#++:     +#+        */
/*   +#+  +#+  +#+    +#+ +#+    +#+ +#+    +#+ +#+  +#+   +#+     +#+     +#+         */
/*  #+#   #+# #+#    #+# #+#    #+# #+#    #+# #+#   #+#  #+#     #+#     #+#          */
/* ###    ### ########  #########   ########  ###    ### ###     ### ###########       */
/*                        <kodokai.featheur@gmail.com>                                 */
/* *********************************************************************************** */

#include "kdo_VkGraphicsPipeline.h"

static void	kdo_readSpirV(Kdo_Vulkan *vk, const char *fileName, Kdo_Shader *shader)
{
	FILE	*file;

	if (!(file = fopen(fileName, "rb")))
	{
		printf("Can't open %s file\n", fileName);
		kdo_cleanup(vk, "File error", 9);
	}
	fseek(file, 0, SEEK_END);
	shader->codeSize = ftell(file);
	rewind(file);

	if (!(shader->code = malloc(shader->codeSize * sizeof(char))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(fread(shader->code, shader->codeSize, 1, file)))
		kdo_cleanup(vk, "Error while reading the shader", 32);

	fclose(file);
}

static void kdo_createShaderModule(Kdo_Vulkan *vk, Kdo_Shader *shader)
{
	VkShaderModuleCreateInfo	moduleInfo;

	moduleInfo.sType		= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo.pNext		= NULL;
	moduleInfo.flags		= 0;
	moduleInfo.codeSize		= shader->codeSize;
	moduleInfo.pCode		= (uint32_t*) shader->code;
	if (vkCreateShaderModule(vk->device.path, &moduleInfo, NULL, &shader->module) != VK_SUCCESS)
		kdo_cleanup(vk, "Create shader module failed", 10);
}

static void	kdo_initPipelineLayout(Kdo_Vulkan *vk)
{
	VkDescriptorSetLayoutBinding				descriptorSetBinding[6];
	VkDescriptorBindingFlags                    bindingFlags[6];
	VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo;
	VkDescriptorSetLayoutCreateInfo				descriptorSetInfo;
	VkPushConstantRange							constantsRange;
	VkPipelineLayoutCreateInfo					pipelineLayoutInfo;

	descriptorSetBinding[0].binding				= 0;
	descriptorSetBinding[0].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetBinding[0].descriptorCount		= 1;
	descriptorSetBinding[0].stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;
	descriptorSetBinding[0].pImmutableSamplers	= NULL;
	bindingFlags[0]                             = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

	descriptorSetBinding[1].binding				= 1;
	descriptorSetBinding[1].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetBinding[1].descriptorCount		= 1;
	descriptorSetBinding[1].stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;
	descriptorSetBinding[1].pImmutableSamplers	= NULL;
	bindingFlags[1]                             = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

	descriptorSetBinding[2].binding				= 2;
	descriptorSetBinding[2].descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorSetBinding[2].descriptorCount		= 1;
	descriptorSetBinding[2].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorSetBinding[2].pImmutableSamplers	= NULL;
	bindingFlags[2]                             = 0;

	descriptorSetBinding[3].binding				= 3;
	descriptorSetBinding[3].descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorSetBinding[3].descriptorCount		= 1;
	descriptorSetBinding[3].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorSetBinding[3].pImmutableSamplers	= NULL;
	bindingFlags[3]                             = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

	descriptorSetBinding[4].binding				= 4;
	descriptorSetBinding[4].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetBinding[4].descriptorCount		= 1;
	descriptorSetBinding[4].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
	bindingFlags[4]                             = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

	descriptorSetBinding[5].binding				= 5;
	descriptorSetBinding[5].descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetBinding[5].descriptorCount		= 1;
	descriptorSetBinding[5].stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorSetBinding[5].pImmutableSamplers	= NULL;
	bindingFlags[5]                             = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;


	bindingFlagsInfo.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	bindingFlagsInfo.pNext          = NULL;
    bindingFlagsInfo.bindingCount   = 6;
    bindingFlagsInfo.pBindingFlags  = bindingFlags;

	descriptorSetInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetInfo.pNext			= &bindingFlagsInfo;
	descriptorSetInfo.flags			= 0;
	descriptorSetInfo.bindingCount	= 6;
	descriptorSetInfo.pBindings		= descriptorSetBinding;
	if (vkCreateDescriptorSetLayout(vk->device.path, &descriptorSetInfo, NULL, &vk->graphicsPipeline.descriptorLayout) != VK_SUCCESS)
		kdo_cleanup(vk, "Descriptor layout creation failed", 11);

	constantsRange.stageFlags	= VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	constantsRange.offset		= 0;
	constantsRange.size			= sizeof(Kdo_VkPush);

	pipelineLayoutInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pNext						= NULL;
	pipelineLayoutInfo.flags						= 0;
	pipelineLayoutInfo.setLayoutCount				= 1;
	pipelineLayoutInfo.pSetLayouts					= &vk->graphicsPipeline.descriptorLayout;
	pipelineLayoutInfo.pushConstantRangeCount		= 1;
	pipelineLayoutInfo.pPushConstantRanges			= &constantsRange;
	if (vkCreatePipelineLayout(vk->device.path, &pipelineLayoutInfo, NULL, &vk->graphicsPipeline.layout) != VK_SUCCESS)
		kdo_cleanup(vk, "Pipeline layout creation failed", 13);
}

void kdo_initGraphicsPipeline(Kdo_Vulkan *vk)
{
	VkGraphicsPipelineCreateInfo				graphicsPipelineInfo;	
	VkPipelineShaderStageCreateInfo				shaderStageInfo[2];
	VkPipelineVertexInputStateCreateInfo		vertexInputInfo;
	VkVertexInputBindingDescription				bindingDescription;
	VkVertexInputAttributeDescription			attributeDescriptions[4];
	VkPipelineInputAssemblyStateCreateInfo		inputAssemblyInfo;
	VkDynamicState								dynamicStates[]	= {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo			dynamicStateInfo;
	VkPipelineViewportStateCreateInfo			viewportStateInfo;
	VkPipelineRasterizationStateCreateInfo		rasterizerInfo;
	VkPipelineMultisampleStateCreateInfo		multisamplingInfo;
	VkPipelineDepthStencilStateCreateInfo		depthStencilInfo;
	VkPipelineColorBlendStateCreateInfo			colorBlendingInfo;
	VkPipelineColorBlendAttachmentState			colorBlendAttachment;

	kdo_initPipelineLayout(vk);

	kdo_readSpirV(vk, "shaders/vertex.spv", &vk->graphicsPipeline.vertexShader);	
	kdo_readSpirV(vk, "shaders/fragment.spv", &vk->graphicsPipeline.fragmentShader);	

	kdo_createShaderModule(vk, &vk->graphicsPipeline.vertexShader);
	kdo_createShaderModule(vk, &vk->graphicsPipeline.fragmentShader);

	shaderStageInfo[0].sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo[0].pNext				= NULL;
	shaderStageInfo[0].flags				= 0;
	shaderStageInfo[0].stage				= VK_SHADER_STAGE_VERTEX_BIT;
	shaderStageInfo[0].module				= vk->graphicsPipeline.vertexShader.module;
	shaderStageInfo[0].pName				= "main";
	shaderStageInfo[0].pSpecializationInfo	= NULL;

	shaderStageInfo[1].sType				= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageInfo[1].pNext				= NULL;
	shaderStageInfo[1].flags				= 0;
	shaderStageInfo[1].stage				= VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStageInfo[1].module				= vk->graphicsPipeline.fragmentShader.module;
	shaderStageInfo[1].pName				= "main";
	shaderStageInfo[1].pSpecializationInfo	= NULL;


	bindingDescription.binding		= 0;
	bindingDescription.stride		= sizeof(Kdo_Vertex);
	bindingDescription.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;

	attributeDescriptions[0].location	= 0;
	attributeDescriptions[0].binding	= 0;
	attributeDescriptions[0].format		= VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset		= offsetof(Kdo_Vertex, pos);

	attributeDescriptions[1].location	= 1;
	attributeDescriptions[1].binding	= 0;
	attributeDescriptions[1].format		= VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset		= offsetof(Kdo_Vertex, normal);

	attributeDescriptions[2].location	= 2;
	attributeDescriptions[2].binding	= 0;
	attributeDescriptions[2].format		= VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset		= offsetof(Kdo_Vertex, tex);

	attributeDescriptions[3].location	= 3;
	attributeDescriptions[3].binding	= 0;
	attributeDescriptions[3].format		= VK_FORMAT_R32_UINT;
	attributeDescriptions[3].offset		= offsetof(Kdo_Vertex, relMaterialIndex);

	vertexInputInfo.sType								= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.pNext								= NULL;
	vertexInputInfo.flags								= 0;
	vertexInputInfo.vertexBindingDescriptionCount		= 1;
	vertexInputInfo.pVertexBindingDescriptions			= &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount		= 4;
	vertexInputInfo.pVertexAttributeDescriptions		= attributeDescriptions;

	inputAssemblyInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.pNext						= NULL;
	inputAssemblyInfo.flags						= 0;
	inputAssemblyInfo.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable	= VK_FALSE;


	dynamicStateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.pNext				= NULL;
	dynamicStateInfo.flags				= 0;
	dynamicStateInfo.dynamicStateCount	= 2;
	dynamicStateInfo.pDynamicStates		= dynamicStates;

	viewportStateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.pNext				= NULL;
	viewportStateInfo.flags				= 0;
	viewportStateInfo.viewportCount		= 1;
	viewportStateInfo.pViewports		= VK_NULL_HANDLE;
	viewportStateInfo.scissorCount		= 1;
	viewportStateInfo.pScissors			= VK_NULL_HANDLE;


	rasterizerInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerInfo.pNext						= NULL;
	rasterizerInfo.flags						= 0;
	rasterizerInfo.depthClampEnable				= VK_FALSE;
	rasterizerInfo.rasterizerDiscardEnable		= VK_FALSE;
	rasterizerInfo.polygonMode					= VK_POLYGON_MODE_FILL;
	rasterizerInfo.cullMode						= VK_CULL_MODE_BACK_BIT; 
	rasterizerInfo.frontFace					= VK_FRONT_FACE_CLOCKWISE;
	rasterizerInfo.depthBiasEnable				= VK_FALSE;
	rasterizerInfo.depthBiasConstantFactor		= 0.0;
	rasterizerInfo.depthBiasClamp				= 0.0;
	rasterizerInfo.depthBiasSlopeFactor			= 0.0;
	rasterizerInfo.lineWidth					= 1.0;

	multisamplingInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.pNext						= NULL;
	multisamplingInfo.flags						= 0;
	multisamplingInfo.rasterizationSamples		= VK_SAMPLE_COUNT_1_BIT;
	multisamplingInfo.sampleShadingEnable		= VK_FALSE;
	multisamplingInfo.minSampleShading			= 1.0;
	multisamplingInfo.pSampleMask				= NULL;
	multisamplingInfo.alphaToCoverageEnable		= VK_FALSE;
	multisamplingInfo.alphaToOneEnable			= VK_FALSE;

	depthStencilInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.pNext					= NULL;
	depthStencilInfo.flags					= 0;
	depthStencilInfo.depthTestEnable		= VK_TRUE;
	depthStencilInfo.depthWriteEnable		= VK_TRUE;
	depthStencilInfo.depthCompareOp			= VK_COMPARE_OP_LESS;
	depthStencilInfo.depthBoundsTestEnable	= VK_FALSE;
	depthStencilInfo.stencilTestEnable		= VK_FALSE;
	depthStencilInfo.front.failOp			= 0;
	depthStencilInfo.front.passOp			= 0;
	depthStencilInfo.front.depthFailOp		= 0;
	depthStencilInfo.front.compareOp		= 0;
	depthStencilInfo.front.compareMask		= 0;
	depthStencilInfo.front.writeMask		= 0;
	depthStencilInfo.front.reference		= 0;
	depthStencilInfo.back.failOp			= 0;
	depthStencilInfo.back.passOp			= 0;
	depthStencilInfo.back.depthFailOp		= 0;
	depthStencilInfo.back.compareOp			= 0;
	depthStencilInfo.back.compareMask		= 0;
	depthStencilInfo.back.writeMask			= 0;
	depthStencilInfo.back.reference			= 0;
	depthStencilInfo.minDepthBounds			= 0.0f;
	depthStencilInfo.maxDepthBounds			= 1.0f;

	colorBlendAttachment.blendEnable			= VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor	= VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor	= VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp			= VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor	= VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor	= VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp			= VK_BLEND_OP_ADD;
	colorBlendAttachment.colorWriteMask		= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	colorBlendingInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendingInfo.pNext					= NULL;
	colorBlendingInfo.flags					= 0;
	colorBlendingInfo.logicOpEnable			= VK_FALSE;
	colorBlendingInfo.logicOp				= VK_LOGIC_OP_COPY;
	colorBlendingInfo.attachmentCount		= 1;
    colorBlendingInfo.pAttachments			= &colorBlendAttachment;
	colorBlendingInfo.blendConstants[0]		= 0.0;
	colorBlendingInfo.blendConstants[1]		= 0.0;
	colorBlendingInfo.blendConstants[2]		= 0.0;
	colorBlendingInfo.blendConstants[3]		= 0.0;

	graphicsPipelineInfo.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineInfo.pNext					= NULL;
	graphicsPipelineInfo.flags					= 0;
	graphicsPipelineInfo.stageCount				= 2;
	graphicsPipelineInfo.pStages				= shaderStageInfo;
	graphicsPipelineInfo.pVertexInputState		= &vertexInputInfo;
	graphicsPipelineInfo.pInputAssemblyState	= &inputAssemblyInfo;
	graphicsPipelineInfo.pTessellationState		= NULL;
	graphicsPipelineInfo.pViewportState			= &viewportStateInfo;
	graphicsPipelineInfo.pRasterizationState	= &rasterizerInfo;
	graphicsPipelineInfo.pMultisampleState		= &multisamplingInfo;
	graphicsPipelineInfo.pDepthStencilState		= &depthStencilInfo;
	graphicsPipelineInfo.pColorBlendState		= &colorBlendingInfo;
	graphicsPipelineInfo.pDynamicState			= &dynamicStateInfo;
	graphicsPipelineInfo.layout					= vk->graphicsPipeline.layout;
	graphicsPipelineInfo.renderPass				= vk->renderPass.path;
	graphicsPipelineInfo.subpass				= 0;
	graphicsPipelineInfo.basePipelineHandle		= VK_NULL_HANDLE;
	graphicsPipelineInfo.basePipelineIndex		= -1;
	if (vkCreateGraphicsPipelines(vk->device.path, VK_NULL_HANDLE, 1, &graphicsPipelineInfo, NULL, &vk->graphicsPipeline.path) != VK_SUCCESS)
			kdo_cleanup(vk, "Graphics pipeline creation failed", 15);

	KDO_FREE(vk->graphicsPipeline.vertexShader.code)
	KDO_FREE(vk->graphicsPipeline.fragmentShader.code)
	KDO_DESTROY(vkDestroyShaderModule, vk->device.path, vk->graphicsPipeline.vertexShader.module)
	KDO_DESTROY(vkDestroyShaderModule, vk->device.path, vk->graphicsPipeline.fragmentShader.module)
}
