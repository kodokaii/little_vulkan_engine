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

static void	kdo_initSampler(Kdo_Vulkan *vk)
{
	VkSamplerCreateInfo		samplerInfo;

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
		kdo_cleanup(vk, "Sampler creation failed", 23);
}

static void kdo_initDescriptorPool(Kdo_Vulkan *vk)
{
	VkDescriptorPoolSize			descriptorPoolSize[2];
	VkDescriptorPoolCreateInfo		descriptorPoolInfo;
	VkDescriptorSetAllocateInfo     allocInfo;

	descriptorPoolSize[0].type				= VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorPoolSize[0].descriptorCount	= 1;

	descriptorPoolSize[1].type				= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorPoolSize[1].descriptorCount	= MAX_MATERIAL;

	descriptorPoolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext			= NULL;
	descriptorPoolInfo.flags			= 0;
	descriptorPoolInfo.maxSets			= 1;
	descriptorPoolInfo.poolSizeCount	= 2;
	descriptorPoolInfo.pPoolSizes		= descriptorPoolSize;
	if (vkCreateDescriptorPool(vk->device.path, &descriptorPoolInfo, NULL, &vk->core.descriptorPool) != VK_SUCCESS)
		kdo_cleanup(vk, "Descriptor pool creation failed", 23);

	  allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	  allocInfo.pNext                 = NULL;
	  allocInfo.descriptorPool        = vk->core.descriptorPool;
	  allocInfo.descriptorSetCount    = 1;
	  allocInfo.pSetLayouts           = &vk->graphicsPipeline.descriptorLayout;

	  if (vkAllocateDescriptorSets(vk->device.path, &allocInfo, &vk->core.objects.descriptorSet) != VK_SUCCESS)
              kdo_cleanup(vk, "Descriptor set allocation failed", 35);

	VkDescriptorImageInfo	imageInfo;
	VkWriteDescriptorSet	descriptorWrite;

	imageInfo.sampler		= vk->core.sampler.basic;
	imageInfo.imageView		= 0;
	imageInfo.imageLayout	= 0;

	descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext				= NULL;
	descriptorWrite.dstSet				= vk->core.objects.descriptorSet;
	descriptorWrite.dstBinding			= 0;
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
		kdo_cleanup(vk, "Transfer pool creation failed", 21);

	vk->core.images.properties.memoryFilter = findTextureMemoryFiltrer(vk);
    vk->core.images.properties.layout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vk->core.images.properties.waitFlags	= WAIT_DEVICE;
    vk->core.vertex.properties.usage		= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vk->core.vertex.properties.waitFlags	= WAIT_DEVICE;
    vk->core.index.properties.usage         = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vk->core.index.properties.waitFlags		= WAIT_DEVICE;

	vk->core.objects.vertex					= &vk->core.vertex;
	vk->core.objects.index					= &vk->core.index;
	vk->core.objects.images					= &vk->core.images;

	kdo_initSampler(vk);	
	kdo_initDescriptorPool(vk);
}
