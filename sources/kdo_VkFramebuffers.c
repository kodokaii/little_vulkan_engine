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

#include "kdo_VkFramebuffers.h"

static void	kdo_initDepthBuffer(Kdo_Vulkan *vk)
{
	Kdo_VkImageCreateInfo	imageInfo;
	VkImageViewCreateInfo   viewInfo;

	imageInfo.path.sType					= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.path.pNext					= NULL;
	imageInfo.path.flags					= 0;
	imageInfo.path.imageType				= VK_IMAGE_TYPE_2D;
	imageInfo.path.format					= vk->framebuffer.depth.format;
	imageInfo.path.extent.width				= vk->swapChain.imagesExtent.width;
	imageInfo.path.extent.height			= vk->swapChain.imagesExtent.height;
	imageInfo.path.extent.depth				= 1;
	imageInfo.path.mipLevels				= 1;
	imageInfo.path.arrayLayers				= 1;
	imageInfo.path.samples					= VK_SAMPLE_COUNT_1_BIT;
	imageInfo.path.tiling					= VK_IMAGE_TILING_OPTIMAL;
	imageInfo.path.usage					= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.path.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.path.queueFamilyIndexCount	= 0;
	imageInfo.path.pQueueFamilyIndices		= NULL;
	imageInfo.path.initialLayout			= VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.memoryFlags					= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	kdo_createImage(vk, imageInfo, &vk->framebuffer.depth.memory, &vk->framebuffer.depth.image);

	viewInfo.sType                              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.pNext                              = NULL;
	viewInfo.flags                              = 0;
	viewInfo.image                              = vk->framebuffer.depth.image;
	viewInfo.viewType                           = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format                             = vk->framebuffer.depth.format;
	viewInfo.components.r                       = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g                       = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b                       = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a                       = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask        = VK_IMAGE_ASPECT_DEPTH_BIT;
	viewInfo.subresourceRange.baseMipLevel      = 0;
	viewInfo.subresourceRange.levelCount        = 1;
	viewInfo.subresourceRange.baseArrayLayer    = 0;
	viewInfo.subresourceRange.layerCount        = 1;
	if (vkCreateImageView(vk->device.path, &viewInfo, NULL, &vk->framebuffer.depth.view) != VK_SUCCESS)
		kdo_cleanup(vk, "View depth image creation failed", 18);
}

void    kdo_initFramebuffers(Kdo_Vulkan *vk)
{
	VkFramebufferCreateInfo	framebufferInfo;

	kdo_initDepthBuffer(vk);

	if (!(vk->framebuffer.path = malloc(vk->swapChain.imagesCount * sizeof(VkFramebuffer))))
		kdo_cleanup(vk, ERRLOC, 12);

	for (uint32_t i = 0; i < vk->swapChain.imagesCount; i++)
	{
		VkImageView	attachments[2]	= {vk->swapChain.views[i], vk->framebuffer.depth.view};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.pNext = NULL;
		framebufferInfo.flags = 0;
		framebufferInfo.renderPass = vk->renderPass.path;
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = vk->swapChain.imagesExtent.width;
		framebufferInfo.height = vk->swapChain.imagesExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(vk->device.path, &framebufferInfo, NULL, vk->framebuffer.path + i) != VK_SUCCESS)
			kdo_cleanup(vk, "Framebuffer creation failed", 19);
	}
}
