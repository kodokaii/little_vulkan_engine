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

#include "kdo_VkGlfw.h"
#include "kdo_VkInit.h"
#include "kdo_VkDisplay.h"
#include "kdo_VkObject.h"

static void provisoire(Kdo_Vulkan *vk)
{
	VkDescriptorSetAllocateInfo     allocInfo;
	VkDescriptorImageInfo			imageInfo;
	VkWriteDescriptorSet			descriptorWrite;

	allocInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext					= NULL;
	allocInfo.descriptorPool		= vk->core.descriptorPool;
	allocInfo.descriptorSetCount	= 1;
	allocInfo.pSetLayouts			= &vk->graphicsPipeline.descriptorLayout;
	if (vkAllocateDescriptorSets(vk->device.path, &allocInfo, &vk->core.descriptorSet) != VK_SUCCESS)
		kdo_cleanup(vk, "Descriptor set allocation failed", 24);

	imageInfo.sampler		= vk->core.basicSampler;
	imageInfo.imageView		= imageView;
	imageInfo.imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext				= NULL;
	descriptorWrite.dstSet				= vk->core.descriptorSet;
	descriptorWrite.dstBinding			= 0;
	descriptorWrite.dstArrayElement		= 0;
	descriptorWrite.descriptorCount		= 1;
	descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.pImageInfo			= &imageInfo;
	descriptorWrite.pBufferInfo			= NULL;
	descriptorWrite.pTexelBufferView	= NULL;
	vkUpdateDescriptorSets(vk->device.path, 1, &descriptorWrite, 0, NULL);
}


int	main(int argc, char *argv[])
{
	Kdo_Vulkan		vk						= {};
	const char		*validationLayers[]		= {"VK_LAYER_KHRONOS_validation"};
	const char		*deviceExtensions[]		= {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	Kdo_VkLoadMeshInfo	info[2];
	Kdo_Vertex			vertex[] = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
};

	char				*texturesPath[]	= {"textures/sky.png", "textures/sky.png"};

	(void) argc;
	(void) argv;

	vk.info.applicationName						= "Vulkan Project";
	vk.info.engineName							= "Kodo Engine";
	vk.info.instanceExtensionsCount				= 0;
	vk.info.instanceExtensions					= NULL;
	vk.info.validationLayersCount				= 1;
	vk.info.validationLayers					= validationLayers;
	vk.info.deviceExtensionsCount				= 1;
	vk.info.deviceExtensions					= deviceExtensions;
	vk.info.windowWidth							= 1000;
	vk.info.windowHeight						= 1000;

	vk.info.queuesInfo[PRESENT_QUEUE].count				= 1;
	vk.info.queuesInfo[PRESENT_QUEUE].priority			= 1.0f;
	vk.info.queuesInfo[PRESENT_QUEUE].requiredFlags		= 0; 
	vk.info.queuesInfo[PRESENT_QUEUE].noRequiredFlags	= 0;
	vk.info.queuesInfo[PRESENT_QUEUE].preferredFlags	= VK_QUEUE_GRAPHICS_BIT;
	vk.info.queuesInfo[PRESENT_QUEUE].noPreferredFlags	= 0;
	vk.info.queuesInfo[PRESENT_QUEUE].presentSupport	= VK_TRUE;

	vk.info.queuesInfo[GRAPHIC_QUEUE].count				= 1;
	vk.info.queuesInfo[GRAPHIC_QUEUE].priority			= 1.0f;
	vk.info.queuesInfo[GRAPHIC_QUEUE].requiredFlags		= VK_QUEUE_GRAPHICS_BIT;
	vk.info.queuesInfo[GRAPHIC_QUEUE].noRequiredFlags	= 0;
	vk.info.queuesInfo[GRAPHIC_QUEUE].preferredFlags	= 0;
	vk.info.queuesInfo[GRAPHIC_QUEUE].noPreferredFlags	= 0;
	vk.info.queuesInfo[GRAPHIC_QUEUE].presentSupport	= 0;

	vk.info.queuesInfo[TRANSFER_QUEUE].count			= 1;
	vk.info.queuesInfo[TRANSFER_QUEUE].priority			= 1.0f;
	vk.info.queuesInfo[TRANSFER_QUEUE].requiredFlags	= VK_QUEUE_GRAPHICS_BIT;
	vk.info.queuesInfo[TRANSFER_QUEUE].noRequiredFlags	= 0;
	vk.info.queuesInfo[TRANSFER_QUEUE].preferredFlags	= 0;
	vk.info.queuesInfo[TRANSFER_QUEUE].noPreferredFlags	= 0;
	vk.info.queuesInfo[TRANSFER_QUEUE].presentSupport	= 0;

	vk.info.presentMode							= VK_PRESENT_MODE_MAILBOX_KHR; 
	vk.info.startPos[0]							= -4.0f;
	vk.info.startPos[1]							= 0.0f;
	vk.info.startPos[2]							= -1.0f;
	vk.info.startYaw							= 0.0f;
	vk.info.startPitch							= 0.0f;

	kdo_initGlfw(&vk);
	kdo_initVulkan(&vk);
	
	info[0].count			= 6;
	info[0].vertex			= vertex;
	info[1].count			= 6;
	info[1].vertex			= vertex;
	kdo_pushMesh(&vk, &vk.core.vertex, &vk.core.index, 1, info);
	kdo_pushTextures(&vk, &vk.core.textures, 1, texturesPath);

	provisoire(&vk);

	kdo_mainLoop(&vk);

	kdo_cleanup(&vk, "Good !", 0);
}
