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

#include "kdo_VkObject.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

static uint32_t	kdo_countUniqueVertex(VkDeviceSize vertexCount, Kdo_Vertex *vertex)
{
	uint32_t i;
	uint32_t j;

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
	Kdo_VkObject				**lastObject;
	Kdo_VkObject				**currentObject;
	int							texChannels;
	uint32_t					i;
	VkDeviceSize				vertexAndIndexSize;
	VkDeviceSize				textureSize;
	VkDeviceSize				textureOffset;
	VkDeviceSize				vertexAndIndexOffset;
	uint32_t					vertexOffset;
	uint32_t					indexOffset;
	void						*data;
	uint32_t					currentIndex;
	stbi_uc						*currentTexture;
	VkBuffer					stagingBuffer;
	VkBuffer					vertexAndIndexBuffer;
	VkDeviceMemory				stagingMemory;
	VkDeviceMemory				vertexAndIndexMemory;
	VkDeviceMemory				textureMemory;
	VkMemoryRequirements		memRequirements;
	VkMemoryAllocateInfo		allocInfo;
	VkCommandBuffer				copyCommandBuffer;
	VkBufferCopy				copyBufferInfo;
	VkImageMemoryBarrier		*imageBarrierInfo;
	VkBufferImageCopy			copyImageInfo;

	lastObject = &vk->render.objects;
	while (*lastObject)
		lastObject = &(*lastObject)->next;

	vertexAndIndexSize	= 0;
	textureSize			= 0;
	currentObject		= lastObject;
	for (i = 0; i < objectsCount; i++)
	{
		if (!(*currentObject = malloc(sizeof(Kdo_VkObject))))
			kdo_cleanup(vk, ERRLOC, 12);

		(*currentObject)->name			= info[i].name;
		(*currentObject)->count			= info[i].count;
		(*currentObject)->status		= info[i].status;
		if (!((*currentObject)->model	= malloc(info[i].count * sizeof(mat4))))
			kdo_cleanup(vk, ERRLOC, 12);
		glm_mat4_identity_array((*currentObject)->model, info[i].count);

		stagingBuffer = kdo_createBuffer(vk, kdo_countUniqueVertex(info[i].vertexCount, info[i].vertex) * sizeof(Kdo_Vertex), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		vkGetBufferMemoryRequirements(vk->device.path, stagingBuffer, &memRequirements);
		(*currentObject)->vertexSize	= memRequirements.size;
		vkDestroyBuffer(vk->device.path, stagingBuffer, NULL);

		stagingBuffer	= kdo_createBuffer(vk, info[i].vertexCount * sizeof(uint32_t), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		vkGetBufferMemoryRequirements(vk->device.path, stagingBuffer, &memRequirements);
		(*currentObject)->indexSize		= memRequirements.size;
		vkDestroyBuffer(vk->device.path, stagingBuffer, NULL);

		stbi_info(info[i].texturePath, &(*currentObject)->textureWidth, &(*currentObject)->textureHeight, &texChannels);
		(*currentObject)->texture	= kdo_createImageTexture(vk, (*currentObject)->textureWidth, (*currentObject)->textureHeight);
		vkGetImageMemoryRequirements(vk->device.path, (*currentObject)->texture, &memRequirements);
		(*currentObject)->textureSize	= memRequirements.size;

		vertexAndIndexSize	+= (*currentObject)->vertexSize + (*currentObject)->indexSize;
		textureSize			+= (*currentObject)->textureSize;

		currentObject = &(*currentObject)->next;
	}

	stagingBuffer = kdo_createBuffer(vk, vertexAndIndexSize + textureSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	vkGetBufferMemoryRequirements(vk->device.path, stagingBuffer, &memRequirements);
	allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext             = NULL;
    allocInfo.allocationSize    = memRequirements.size;
    allocInfo.memoryTypeIndex   = kdo_findMemoryType(vk, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (vkAllocateMemory(vk->device.path, &allocInfo, NULL, &stagingMemory) != VK_SUCCESS)
		kdo_cleanup(vk, "allocation staging memory failed", 25);
	vkBindBufferMemory(vk->device.path, stagingBuffer, stagingMemory, 0);

	vkMapMemory(vk->device.path, stagingMemory, 0, VK_WHOLE_SIZE, 0, &data);

	vertexAndIndexOffset	= 0;
	textureOffset			= vertexAndIndexSize;
	currentObject			= lastObject;
	for (i = 0; i < objectsCount; i++)
	{
		vertexOffset	= 0;
		indexOffset		= 0;
		for (uint32_t j = 0; j < info[i].vertexCount; j++)
		{
			for (currentIndex = 0; currentIndex < j 
						&& (!glm_vec3_eqv(info[i].vertex[currentIndex].pos, info[i].vertex[j].pos) 
						|| !glm_vec3_eqv(info[i].vertex[currentIndex].color, info[i].vertex[j].tex) 
						|| !glm_vec2_eqv(info[i].vertex[currentIndex].tex, info[i].vertex[j].tex)); currentIndex++);
			if (currentIndex < j)
				memcpy(data + vertexAndIndexOffset + (*currentObject)->vertexSize + (indexOffset++ * sizeof(uint32_t)), &currentIndex, sizeof(uint32_t));
			else
				memcpy(data + vertexAndIndexOffset + (vertexOffset++ * sizeof(Kdo_Vertex)), info[i].vertex + j, sizeof(Kdo_Vertex));
		}
		vertexAndIndexOffset += (*currentObject)->vertexSize + (*currentObject)->indexSize;

		if (!(currentTexture   = stbi_load(info[i].texturePath, &(*currentObject)->textureWidth, &(*currentObject)->textureHeight, &texChannels, STBI_rgb_alpha)))
			kdo_cleanup(vk, "Texture load failed", 26);
		memcpy(data + textureOffset, currentTexture, (*currentObject)->textureWidth * (*currentObject)->textureHeight * 4);
		stbi_image_free(currentTexture);
		textureOffset += (*currentObject)->textureSize;

		currentObject = &(*currentObject)->next;
	}
	vkUnmapMemory(vk->device.path, stagingMemory);

	vertexAndIndexBuffer = kdo_createBuffer(vk, vertexAndIndexSize + vk->render.vertexAndIndexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	vkGetBufferMemoryRequirements(vk->device.path, vertexAndIndexBuffer, &memRequirements);
    allocInfo.allocationSize    = memRequirements.size;
    allocInfo.memoryTypeIndex   = kdo_findMemoryType(vk, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (vkAllocateMemory(vk->device.path, &allocInfo, NULL, &vertexAndIndexMemory) != VK_SUCCESS)
		kdo_cleanup(vk, "allocation vertex and index memory failed", 27);
	vkBindBufferMemory(vk->device.path, vertexAndIndexBuffer, vertexAndIndexMemory, 0);

	allocInfo.allocationSize	= textureSize + vk->render.textureSize;
	allocInfo.memoryTypeIndex   = vk->render.textureMemoryType;
	if (vkAllocateMemory(vk->device.path, &allocInfo, NULL, &textureMemory) != VK_SUCCESS)
		kdo_cleanup(vk, "allocation texture memory failed", 28);


	kdo_beginUniqueCommand(vk, &copyCommandBuffer);

	if (vk->render.vertexAndIndexBuffer)
	{
		copyBufferInfo.srcOffset	= 0;
		copyBufferInfo.dstOffset	= 0;
		copyBufferInfo.size			= vk->render.vertexAndIndexSize;
		vkCmdCopyBuffer(copyCommandBuffer, vk->render.vertexAndIndexBuffer, vertexAndIndexBuffer, 1, &copyBufferInfo);
	}
	
	copyBufferInfo.srcOffset	= 0;
	copyBufferInfo.dstOffset	= vk->render.vertexAndIndexSize;
	copyBufferInfo.size			= vertexAndIndexSize;
	vkCmdCopyBuffer(copyCommandBuffer, stagingBuffer, vertexAndIndexBuffer, 1, &copyBufferInfo);

	if (!(imageBarrierInfo = malloc(objectsCount * sizeof(VkImageMemoryBarrier))))
		kdo_cleanup(vk, ERRLOC, 12);

	textureOffset	= 0;
	currentObject	= lastObject;
	for (i = 0; i < objectsCount; i++)
	{
		vkBindImageMemory(vk->device.path, (*currentObject)->texture, textureMemory, textureOffset);

		imageBarrierInfo[i].sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrierInfo[i].pNext							= NULL;
		imageBarrierInfo[i].srcAccessMask					= 0;
		imageBarrierInfo[i].dstAccessMask					= VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrierInfo[i].oldLayout						= VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrierInfo[i].newLayout						= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrierInfo[i].srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
		imageBarrierInfo[i].dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
		imageBarrierInfo[i].image							= (*currentObject)->texture;
		imageBarrierInfo[i].subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		imageBarrierInfo[i].subresourceRange.baseMipLevel	= 0;
		imageBarrierInfo[i].subresourceRange.levelCount		= 1;
		imageBarrierInfo[i].subresourceRange.baseArrayLayer	= 0;
		imageBarrierInfo[i].subresourceRange.layerCount		= 1;
	
		textureOffset += (*currentObject)->textureSize;
		currentObject = &(*currentObject)->next;
	}
	vkCmdPipelineBarrier(copyCommandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, objectsCount, imageBarrierInfo);

	textureOffset	= vertexAndIndexSize;
	currentObject	= lastObject;
	for (i = 0; i < objectsCount; i++)
	{
		copyImageInfo.bufferOffset							= textureOffset;
		copyImageInfo.bufferRowLength						= 0;
		copyImageInfo.bufferImageHeight						= 0;
		copyImageInfo.imageSubresource.aspectMask			= VK_IMAGE_ASPECT_COLOR_BIT;
		copyImageInfo.imageSubresource.mipLevel				= 0;
		copyImageInfo.imageSubresource.baseArrayLayer		= 0;
		copyImageInfo.imageSubresource.layerCount			= 1;
		copyImageInfo.imageOffset.x							= 0;
		copyImageInfo.imageOffset.y							= 0;
		copyImageInfo.imageOffset.z							= 0;
		copyImageInfo.imageExtent.width						= (*currentObject)->textureWidth;
		copyImageInfo.imageExtent.height					= (*currentObject)->textureHeight;
		copyImageInfo.imageExtent.depth						= 1;
		vkCmdCopyBufferToImage(copyCommandBuffer, stagingBuffer, (*currentObject)->texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyImageInfo);

		imageBarrierInfo[i].srcAccessMask	= VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrierInfo[i].dstAccessMask	= VK_ACCESS_SHADER_READ_BIT;
		imageBarrierInfo[i].oldLayout		= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrierInfo[i].newLayout		= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		textureOffset += (*currentObject)->textureSize;
		currentObject = &(*currentObject)->next;
	}
	vkCmdPipelineBarrier(copyCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, objectsCount, imageBarrierInfo);

	kdo_endUniqueCommand(vk, &copyCommandBuffer);
	vkDestroyBuffer(vk->device.path, stagingBuffer, NULL);
	vkFreeMemory(vk->device.path, stagingMemory, NULL);
	free(imageBarrierInfo);


	vkDeviceWaitIdle(vk->device.path);
	KDO_DESTROY(vkDestroyBuffer, vk->device.path, vk->render.vertexAndIndexBuffer)
	KDO_DESTROY(vkFreeMemory, vk->device.path, vk->render.vertexAndIndexMemory)
	KDO_DESTROY(vkFreeMemory, vk->device.path, vk->render.textureMemory)
	vk->render.vertexAndIndexBuffer	= vertexAndIndexBuffer;
	vk->render.vertexAndIndexMemory	= vertexAndIndexMemory;
	vk->render.textureMemory		= textureMemory;
	vk->render.vertexAndIndexSize	+= vertexAndIndexSize;
	vk->render.textureSize			+= textureSize;
	vk->render.objectsCount			+= objectsCount;
}
