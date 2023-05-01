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

void	kdo_queueTransferBuffer(Kdo_Vulkan *vk, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, VkBuffer buffer)
{
	VkCommandBuffer			commandBuffer;
	VkBufferMemoryBarrier	bufferBarrierInfo;
	
	kdo_beginUniqueCommand(vk, &commandBuffer);

	bufferBarrierInfo.sType					= VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	bufferBarrierInfo.pNext					= NULL;
	bufferBarrierInfo.srcAccessMask			= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	bufferBarrierInfo.dstAccessMask			= VK_PIPELINE_STAGE_TRANSFER_BIT;
	bufferBarrierInfo.srcQueueFamilyIndex	= srcQueueFamilyIndex;
	bufferBarrierInfo.dstQueueFamilyIndex	= dstQueueFamilyIndex;
	bufferBarrierInfo.buffer				= buffer;
	bufferBarrierInfo.offset				= 0;
	bufferBarrierInfo.size					= VK_WHOLE_SIZE;
	vkCmdPipelineBarrier(commandBuffer, 0, VK_ACCESS_TRANSFER_WRITE_BIT, 0, 0, NULL, 1, &bufferBarrierInfo, 0, NULL);

	kdo_endUniqueCommand(vk, &commandBuffer);
}

VkBuffer	kdo_createBuffer(Kdo_Vulkan *vk, VkDeviceSize size, VkBufferUsageFlags usage)
{
	VkBufferCreateInfo	bufferInfo;
	VkBuffer			buffer;

	bufferInfo.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext					= NULL;
	bufferInfo.flags					= 0;
	bufferInfo.size						= size;
	bufferInfo.usage					= usage;
	bufferInfo.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.queueFamilyIndexCount	= 0;
	bufferInfo.pQueueFamilyIndices		= NULL;
	if (vkCreateBuffer(vk->device.path, &bufferInfo, NULL, &buffer) != VK_SUCCESS)
		kdo_cleanup(vk, "Buffer creation failed", 24);

	return (buffer);
}

VkImage	kdo_createImageTexture(Kdo_Vulkan *vk, int width, int height)
{
	VkImageCreateInfo	imageInfo;
	VkImage				image;

	imageInfo.sType					= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext					= NULL;
	imageInfo.flags					= 0;
	imageInfo.imageType				= VK_IMAGE_TYPE_2D;
	imageInfo.format				= VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.extent.width			= width;
	imageInfo.extent.height			= height;
	imageInfo.extent.depth			= 1;
	imageInfo.mipLevels				= 1;
	imageInfo.arrayLayers			= 1;
	imageInfo.samples				= VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling				= VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage					= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode			= VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.queueFamilyIndexCount	= 0;
	imageInfo.pQueueFamilyIndices	= NULL;
	imageInfo.initialLayout			= VK_IMAGE_LAYOUT_UNDEFINED;
	if (vkCreateImage(vk->device.path, &imageInfo, NULL, &image) != VK_SUCCESS)
		kdo_cleanup(vk, "Image texture creation failed", 25);

	return (image);
}
