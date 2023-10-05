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

#include "kdo_VkCore.h"

static void kdo_initDescriptorPool(Kdo_Vulkan *vk)
{
	VkDescriptorPoolSize			descriptorPoolSize[7];
	VkDescriptorPoolCreateInfo		descriptorPoolInfo;
	VkDescriptorSetAllocateInfo     allocInfo;

	descriptorPoolSize[0].type				= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorPoolSize[0].descriptorCount	= 1;

	descriptorPoolSize[1].type				= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorPoolSize[1].descriptorCount	= 1;

	descriptorPoolSize[2].type				= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorPoolSize[2].descriptorCount	= 1;

	descriptorPoolSize[3].type				= VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorPoolSize[3].descriptorCount	= 1;

	descriptorPoolSize[4].type				= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorPoolSize[4].descriptorCount	= MAX_TEXTURE;

	descriptorPoolSize[5].type				= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorPoolSize[5].descriptorCount	= 1;

	descriptorPoolSize[6].type				= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorPoolSize[6].descriptorCount	= 1;

	descriptorPoolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext			= NULL;
	descriptorPoolInfo.flags			= 0;
	descriptorPoolInfo.maxSets			= 1;
	descriptorPoolInfo.poolSizeCount	= 7;
	descriptorPoolInfo.pPoolSizes		= descriptorPoolSize;
	if (vkCreateDescriptorPool(vk->device.path, &descriptorPoolInfo, NULL, &vk->core.descriptorPool) != VK_SUCCESS)
		kdo_cleanup(vk, "Descriptor pool creation failed", 37);

	allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext                 = NULL;
	allocInfo.descriptorPool        = vk->core.descriptorPool;
	allocInfo.descriptorSetCount    = 1;
	allocInfo.pSetLayouts           = &vk->graphicsPipeline.descriptorLayout;
	if (vkAllocateDescriptorSets(vk->device.path, &allocInfo, &vk->core.descriptorSet) != VK_SUCCESS)
              kdo_cleanup(vk, "Descriptor set allocation failed", 38);
}

static void kdo_initBuffer(Kdo_Vulkan *vk)
{
	uint32_t		indexNull;
	vec3			defaultVec3		=	{0, 0, 0};
	vec2			defaultVec2		=	{0, 0};
	Kdo_VkMaterial  defaultMtl		=  {{0, 0, 0}, 0,  //ambient
										{1, 1, 1}, 0,   //diffuse
										{1, 1, 1}, 0,   //specular
										{1, 1, 1}, 0,   //emissive
										{1, 1, 1}, 0,   //transmittance
										{1, 1, 1}, 0,   //transmissionFilter
										400, 0,         //shininess
										1, 0,           //refractionIndex
										1, 0,           //disolve
										1,              //illum
										0};             //bumpMap
	Kdo_VkLight		defaultLight	=  {{5.0f, 0.0f, -10.0f, 10.0f},
									    {1.0f, 1.0f, 1.0f, 1.0f}};
	char			*defaultTexture;

	KDO_VK_ALLOC(defaultTexture, strdup("textures/default.png"));

	kdo_vkNewSetBuffer(vk, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(vec3), sizeof(vec3), &vk->core.buffer.vector3);
	kdo_vkNewSetBuffer(vk, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(vec2), sizeof(vec2), &vk->core.buffer.vector2);
	kdo_vkNewSetBuffer(vk, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(Kdo_VkMaterial), sizeof(Kdo_VkMaterial), &vk->core.buffer.material);
	kdo_vkNewSetImageBuffer(vk, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MAX_TEXTURE, 128, &vk->core.buffer.texture);
	kdo_vkNewBuffer(vk, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(Kdo_VkLight), &vk->core.buffer.light);
	kdo_vkNewBuffer(vk, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(Kdo_VkVertex), &vk->core.buffer.vertex);
	kdo_vkNewBuffer(vk, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(Kdo_VkObject), &vk->core.buffer.object);

	kdo_vkPushSetBufferData(vk, &vk->core.buffer.vector3, defaultVec3, &indexNull);
	kdo_vkPushSetBufferData(vk, &vk->core.buffer.vector2, defaultVec2, &indexNull);
	kdo_vkPushSetBufferData(vk, &vk->core.buffer.material, &defaultMtl, &indexNull);
	kdo_vkPushSetImageBufferPath(vk, &vk->core.buffer.texture, defaultTexture, &indexNull);
	kdo_vkPushBufferData(vk, &vk->core.buffer.light, &defaultLight, sizeof(Kdo_VkLight));
}

static void	kdo_initSampler(Kdo_Vulkan *vk)
{
	VkSamplerCreateInfo		samplerInfo;
	VkDescriptorImageInfo	imageInfo;
	VkWriteDescriptorSet	descriptorWrite;


	samplerInfo.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext					= NULL;
	samplerInfo.flags					= 0;
	samplerInfo.magFilter				= VK_FILTER_LINEAR;
	samplerInfo.minFilter				= VK_FILTER_LINEAR;
	samplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias				= 0.0f;
	samplerInfo.anisotropyEnable		= VK_TRUE;
	samplerInfo.maxAnisotropy			= 16.0f;
	samplerInfo.compareEnable			= VK_FALSE;
	samplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
	samplerInfo.minLod					= 0.0f;
	samplerInfo.maxLod					= 0.0f;
	samplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates	= VK_FALSE;
	if (vkCreateSampler(vk->device.path, &samplerInfo, NULL, &vk->core.sampler.basic) != VK_SUCCESS)
		kdo_cleanup(vk, "Sampler creation failed", 30);

	imageInfo.sampler		= vk->core.sampler.basic;
	imageInfo.imageView		= 0;
	imageInfo.imageLayout	= 0;

	descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext				= NULL;
	descriptorWrite.dstSet				= vk->core.descriptorSet;
	descriptorWrite.dstBinding			= 3;
	descriptorWrite.dstArrayElement		= 0;
	descriptorWrite.descriptorCount		= 1;
	descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorWrite.pImageInfo			= &imageInfo;
	descriptorWrite.pBufferInfo			= NULL;
	descriptorWrite.pTexelBufferView	= NULL;
	vkUpdateDescriptorSets(vk->device.path, 1, &descriptorWrite, 0, NULL);
}

void	kdo_initCore(Kdo_Vulkan *vk)
{
	VkCommandPoolCreateInfo	commandPoolInfo;

	commandPoolInfo.sType               = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext               = NULL;
	commandPoolInfo.flags               = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolInfo.queueFamilyIndex    = vk->device.queues[TRANSFER_QUEUE].familyIndex;
	if (vkCreateCommandPool(vk->device.path, &commandPoolInfo, NULL, &vk->core.transferPool) != VK_SUCCESS)
		kdo_cleanup(vk, "Transfer pool creation failed", 22);

	kdo_initDescriptorPool(vk);
	kdo_initBuffer(vk);
	kdo_initSampler(vk);	
}
