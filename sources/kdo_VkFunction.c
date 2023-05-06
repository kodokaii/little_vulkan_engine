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

#include "kdo_VkFunction.h"

int	kdo_checkExtensions(const char **find, uint32_t findCount, const char *list, uint32_t listCount, size_t offsetList)
{
	uint32_t	j;

	for (uint32_t i = 0; i < findCount; i++)
	{
		for (j = 0; j < listCount && strcmp(find[i], list + j * offsetList); j++);
		if (j == listCount)
		{
			printf("%s is missing\n", find[i]);
			return (1);
		}
	}
	return (0);
}

VkFormat	kdo_findFormat(Kdo_Vulkan *vk, VkFormat *formats, uint32_t formatsCount, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	VkFormatProperties	properties;

	for (uint32_t i = 0; i < formatsCount; i++)
	{
		vkGetPhysicalDeviceFormatProperties(vk->physicalDevice.path, formats[i], &properties);
		if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
			return (formats[i]);
		if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
			return (formats[i]);
	}
	kdo_cleanup(vk, "Couldn't find format", 8);
	return (FOR_NO_ERROR);
}

uint32_t	kdo_findMemoryType(Kdo_Vulkan *vk, uint32_t typeFilter, VkMemoryPropertyFlags memoryFlags)
{
	for (uint32_t i = 0; i < vk->physicalDevice.memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (vk->physicalDevice.memProperties.memoryTypes[i].propertyFlags & memoryFlags) == memoryFlags)
			return (i);
	}
	kdo_cleanup(vk, "Couldn't find a suitable memory type", 15);
	return (FOR_NO_ERROR);
}

void	kdo_beginUniqueCommand(Kdo_Vulkan *vk, VkCommandBuffer *commandBuffer)
{
	VkCommandBufferAllocateInfo	allocInfo;
	VkCommandBufferBeginInfo	beginInfo;

	allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext					= NULL;
	allocInfo.commandPool			= vk->render.transferPool;
	allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount	= 1;
	if (vkAllocateCommandBuffers(vk->device.path, &allocInfo, commandBuffer) != VK_SUCCESS)
		kdo_cleanup(vk, "CommandBuffer allocation failed", 22);

	beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext				= NULL;
	beginInfo.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo	= NULL;
	vkBeginCommandBuffer(*commandBuffer, &beginInfo);
}

void	kdo_endUniqueCommand(Kdo_Vulkan *vk, VkCommandBuffer *commandBuffer)
{
	VkSubmitInfo	submitInfo;

	vkEndCommandBuffer(*commandBuffer);	

	submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext				= NULL;
	submitInfo.waitSemaphoreCount	= 0;
	submitInfo.pWaitSemaphores		= NULL;
	submitInfo.pWaitDstStageMask	= NULL;
	submitInfo.commandBufferCount	= 1;
	submitInfo.pCommandBuffers		= commandBuffer;
	submitInfo.signalSemaphoreCount	= 0;
	submitInfo.pSignalSemaphores	= NULL;
	vkQueueSubmit(vk->device.queues[TRANSFER_QUEUE].path, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(vk->device.queues[TRANSFER_QUEUE].path);

	vkFreeCommandBuffers(vk->device.path, vk->render.transferPool, 1, commandBuffer);
}

void	kdo_imageTextureInfo(VkExtent3D extent, VkImageCreateInfo *imageInfo)
{
	imageInfo->sType					= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo->pNext					= NULL;
	imageInfo->flags					= 0;
	imageInfo->imageType				= VK_IMAGE_TYPE_2D;
	imageInfo->format					= VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo->extent					= extent;
	imageInfo->mipLevels				= 1;
	imageInfo->arrayLayers				= 1;
	imageInfo->samples					= VK_SAMPLE_COUNT_1_BIT;
	imageInfo->tiling					= VK_IMAGE_TILING_OPTIMAL;
	imageInfo->usage					= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo->sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	imageInfo->queueFamilyIndexCount	= 0;
	imageInfo->pQueueFamilyIndices		= NULL;
	imageInfo->initialLayout			= VK_IMAGE_LAYOUT_UNDEFINED;
}

uint32_t	findTextureMemoryFiltrer(Kdo_Vulkan *vk)
{
	VkImageCreateInfo		imageInfo;
	VkExtent3D				extent	= {vk->physicalDevice.properties.limits.maxImageDimension2D, vk->physicalDevice.properties.limits.maxImageDimension2D, 1};
	VkImage					image;
	VkMemoryRequirements	memRequirements;

	kdo_imageTextureInfo(extent, &imageInfo);
	vkCreateImage(vk->device.path, &imageInfo, NULL, &image);
	vkGetImageMemoryRequirements(vk->device.path, image, &memRequirements);
	vkDestroyImage(vk->device.path, image, NULL);

	return (memRequirements.memoryTypeBits);
}

void	*kdo_mallocMerge(size_t sizeSrc1, void *src1, size_t sizeSrc2, void *src2)
{
	void	*dst;

	dst = malloc(sizeSrc1 + sizeSrc2);
	memcpy(dst, src1, sizeSrc1); 
	memcpy(dst + sizeSrc1, src2, sizeSrc2);

	return (dst);
}
