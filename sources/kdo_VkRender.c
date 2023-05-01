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

#include "kdo_VkRender.h"

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
	if (vkCreateSampler(vk->device.path, &samplerInfo, NULL, &vk->render.basicSampler) != VK_SUCCESS)
		kdo_cleanup(vk, "Sampler creation failed", 23);
}

static void kdo_initDescriptor(Kdo_Vulkan *vk)
{
	VkDescriptorPoolSize			descriptorPoolSize;
	VkDescriptorPoolCreateInfo		descriptorPoolInfo;
	VkDescriptorSetAllocateInfo     allocInfo;

	descriptorPoolSize.type				= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorPoolSize.descriptorCount	= 1;

	descriptorPoolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext			= NULL;
	descriptorPoolInfo.flags			= 0;
	descriptorPoolInfo.maxSets			= 1;
	descriptorPoolInfo.poolSizeCount	= 1;
	descriptorPoolInfo.pPoolSizes		= &descriptorPoolSize;
	if (vkCreateDescriptorPool(vk->device.path, &descriptorPoolInfo, NULL, &vk->render.descriptorPool) != VK_SUCCESS)
		kdo_cleanup(vk, "Descriptor pool creation failed", 23);

	allocInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext					= NULL;
	allocInfo.descriptorPool		= vk->render.descriptorPool;
	allocInfo.descriptorSetCount	= 1;
	allocInfo.pSetLayouts			= &vk->graphicsPipeline.descriptorLayout;
	if (vkAllocateDescriptorSets(vk->device.path, &allocInfo, &vk->render.descriptorSet) != VK_SUCCESS)
		kdo_cleanup(vk, "Descriptor set allocation failed", 24);
}

static void	kdo_initMemoryType(Kdo_Vulkan *vk)
{
	VkMemoryRequirements	memRequirements;
	VkImage					image;

	image	= kdo_createImageTexture(vk, vk->physicalDevice.properties.limits.maxImageDimension2D, vk->physicalDevice.properties.limits.maxImageDimension2D);
	vkGetImageMemoryRequirements(vk->device.path, image, &memRequirements);
	vk->render.textureMemoryType = kdo_findMemoryType(vk, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	vkDestroyImage(vk->device.path, image, NULL);
}

void	kdo_initRender(Kdo_Vulkan *vk)
{
	VkCommandPoolCreateInfo	commandPoolInfo;

	commandPoolInfo.sType               = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext               = NULL;
	commandPoolInfo.flags               = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolInfo.queueFamilyIndex    = vk->device.queues[TRANSFER_QUEUE].familyIndex;
	if (vkCreateCommandPool(vk->device.path, &commandPoolInfo, NULL, &vk->render.transferPool) != VK_SUCCESS)
		kdo_cleanup(vk, "Transfer pool creation failed", 21);

	kdo_initSampler(vk);	
	kdo_initDescriptor(vk);
	kdo_initMemoryType(vk);
}
