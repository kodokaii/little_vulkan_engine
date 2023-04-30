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

uint32_t	kdo_findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags memoryFlags, Kdo_Vulkan *vk)
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
	vkQueueSubmit(vk->device.queues[GRAPHIC_QUEUE].path, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(vk->device.queues[GRAPHIC_QUEUE].path);

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

VkDeviceSize	kdo_countUniqueVertex(VkDeviceSize vertexCount, Kdo_Vertex *vertex)
{
	VkDeviceSize i;
	VkDeviceSize j;

	for (i = 0; i < vertexCount; i++)
	{
		for (j = i + 1; j < vertexCount
				 &&(!glm_vec3_eqv(vertex[i].pos, vertex[j].pos)
				 || !glm_vec3_eqv(vertex[i].color, vertex[j].color)
				 || !glm_vec2_eqv(vertex[i].tex, vertex[j].tex)); j++);
		if (j != vertexCount)
			vertexCount--;
	}
	return (vertexCount);
}

void	kdo_addObjects(Kdo_Vulkan *vk, uint32_t objectsCount, Kdo_VkAddObjectInfo *info)
{
	Kdo_VkObject	**lastObject;
	Kdo_VkObject	**currentObject;
	VkDeviceSize	sizeVertexAndIndex	= 0;
	VkDeviceSize	sizeTexture			= 0;
	VkDeviceMemory	stagingMemoryVertexAndIndex;
	VkDeviceMemory	stagingMemoryTexture;


	lastObject = &vk->render.objects;
	while (*lastObject)
		lastObject = &(*lastObject)->next;

	currentObject = lastObject;
	for (uint32_t i = 0; i < objectsCount; i++)
	{
		if (!(*currentObject = malloc(sizeof(Kdo_VkObject))))
			kdo_cleanup(vk, ERRLOC, 12);

		(*currentObject)->name			= info[i].name;
		(*currentObject)->count			= info[i].count;
		(*currentObject)->status		= info[i].status;
		(*currentObject)->vertexCount	= kdo_countUniqueVertex(info[i].vertexCount, info[i].vertex);
		(*currentObject)->indexCount	= info[i].vertexCount;

		//textureSize

		sizeVertexAndIndex += (*currentObject)->vertexCount + (*currentObject)->indexCount;

		currentObject = &(*currentObject)->next;
	}
}
