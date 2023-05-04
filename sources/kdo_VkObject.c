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

static uint32_t	kdo_vertexEqv(Kdo_Vertex vertex1, Kdo_Vertex vertex2)
{
	return (glm_vec3_eqv_eps(vertex1.pos, vertex2.pos) || glm_vec3_eqv_eps(vertex1.color, vertex2.color) || glm_vec2_eqv_eps(vertex1.tex, vertex2.tex));
}

static uint32_t	kdo_countUniqueVertex(VkDeviceSize vertexCount, Kdo_Vertex *vertex)
{
	uint32_t i;
	uint32_t j;

	for (i = 0; i < vertexCount; i++)
	{
		for (j = i + 1; j < vertexCount && !kdo_vertexEqv(vertex[i], vertex[j]); j++);
		if (j != vertexCount)
			vertexCount--;
	}
	return (vertexCount);
}

void	kdo_allocBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer, VkMemoryPropertyFlags memoryFlags)
{
	VkBufferCreateInfo		bufferInfo;
	VkMemoryRequirements	memRequirements;
	VkMemoryAllocateInfo	allocInfo;

	bufferInfo.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext					= NULL;
	bufferInfo.flags					= 0;
	bufferInfo.size						= buffer->size;
	bufferInfo.usage					= buffer->usage;
	bufferInfo.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.queueFamilyIndexCount	= 0;
	bufferInfo.pQueueFamilyIndices		= NULL;
	if (vkCreateBuffer(vk->device.path, &bufferInfo, NULL, &buffer->buffer) != VK_SUCCESS)
		kdo_cleanup(vk, "Buffer creation failed", 24);
	
	vkGetBufferMemoryRequirements(vk->device.path, buffer->buffer, &memRequirements);

	if ((buffer->size = memRequirements.size))
	{
		allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext             = NULL;
		allocInfo.allocationSize    = memRequirements.size;
		allocInfo.memoryTypeIndex   = kdo_findMemoryType(vk, memRequirements.memoryTypeBits, memoryFlags);
		if (vkAllocateMemory(vk->device.path, &allocInfo, NULL, &buffer->memory) != VK_SUCCESS)
			kdo_cleanup(vk, "allocation vertex and index memory failed", 26);
		vkBindBufferMemory(vk->device.path, buffer->buffer, buffer->memory, 0);
	}
}

Kdo_VkBuffer	kdo_mergeBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferSrc1, Kdo_VkBuffer *bufferSrc2)
{
	Kdo_VkBuffer			bufferDst = {};
	VkCommandBuffer			copy;
	VkBufferCopy			copyInfo;
	Kdo_VkBufferDiv			**last;

	bufferDst.usage			= bufferSrc1->usage | bufferSrc2->usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bufferDst.size			= bufferSrc1->size + bufferSrc2->size;
	bufferDst.divCount		= bufferSrc1->divCount + bufferSrc2->divCount;
	bufferDst.div			= bufferSrc1->div;
	last	= &bufferDst.div;
	while (*last)
		last = &(*last)->next;
	*last	= bufferSrc2->div;

	kdo_allocBuffer(vk, &bufferDst, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	kdo_beginUniqueCommand(vk, &copy);
	copyInfo.srcOffset	= 0;

	if (bufferSrc1->size)
	{
		copyInfo.dstOffset	= 0;
		copyInfo.size		= bufferSrc1->size;
		vkCmdCopyBuffer(copy, bufferSrc1->buffer, bufferDst.buffer, 1, &copyInfo);
	}

	if (bufferSrc2->size)
	{
		copyInfo.srcOffset	= 0;
		copyInfo.dstOffset	= bufferSrc1->size;
		copyInfo.size		= bufferSrc2->size;
		vkCmdCopyBuffer(copy, bufferSrc2->buffer, bufferDst.buffer, 1, &copyInfo);
	}
	kdo_endUniqueCommand(vk, &copy);

	KDO_DESTROY(vkDestroyBuffer, vk->device.path, bufferSrc1->buffer)
	KDO_DESTROY(vkFreeMemory, vk->device.path, bufferSrc1->memory)
	bufferSrc1->usage		= 0;
	bufferSrc1->size		= 0;
	bufferSrc1->divCount	= 0;
	bufferSrc1->div			= NULL;

	KDO_DESTROY(vkFreeMemory, vk->device.path, bufferSrc2->memory)
	KDO_DESTROY(vkDestroyBuffer, vk->device.path, bufferSrc2->buffer)
	bufferSrc2->usage		= 0;
	bufferSrc2->size		= 0;
	bufferSrc2->divCount	= 0;
	bufferSrc2->div			= NULL;

	return (bufferDst);
	}

void	kdo_addData(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferDst, uint32_t infoCount, Kdo_VkAddDataInfo *info)
{
	Kdo_VkBuffer	stagingBuffer = {};
	Kdo_VkBufferDiv	**current;
	VkDeviceSize	offset;
	uint32_t		i;
	void			*data;

	stagingBuffer.size	= 0;
	current				= &stagingBuffer.div;
	for (i = 0; i < infoCount; i++)
	{
		if (!(*current = malloc(sizeof(Kdo_VkBufferDiv))))
			kdo_cleanup(vk, ERRLOC, 12);
		(*current)->count		= info[i].count;
		(*current)->size		= info[i].size;
		(*current)->next		= NULL;
		current					= &(*current)->next;

		stagingBuffer.size += info[i].count * info[i].size;
	}
	stagingBuffer.usage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT; 
	stagingBuffer.divCount	= infoCount;

	kdo_allocBuffer(vk, &stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	offset = 0;
	vkMapMemory(vk->device.path, stagingBuffer.memory, 0, VK_WHOLE_SIZE, 0, &data);
	for (i = 0; i < infoCount; i++)
	{
		memcpy(data + offset, info[i].data, info[i].count * info[i].size);
		offset += info[i].count * info[i].size;
	}
	vkUnmapMemory(vk->device.path, stagingBuffer.memory);
	
	*bufferDst = kdo_mergeBuffer(vk, bufferDst, &stagingBuffer);
}

void	kdo_loadMesh(Kdo_Vulkan *vk, Kdo_VkBuffer *vertexBuffer, Kdo_VkBuffer *indexBuffer, uint32_t infoCount, Kdo_VkLoadMeshInfo *info)
{
	Kdo_VkAddDataInfo	*vertexInfo;
	Kdo_VkAddDataInfo	*indexInfo;
	uint32_t			currentIndex;
	uint32_t			indexCount;
	uint32_t			vertexCount;
	uint32_t			i;
	uint32_t			j;

	if (!(vertexInfo = malloc(infoCount * sizeof(Kdo_VkAddDataInfo))))
		kdo_cleanup(vk, ERRLOC, 12);	
	if (!(indexInfo = malloc(infoCount * sizeof(Kdo_VkAddDataInfo))))
		kdo_cleanup(vk, ERRLOC, 12);	

	for (i = 0; i < infoCount; i++)
	{
		indexInfo[i].count	= info[i].count;
		indexInfo[i].size	= sizeof(uint32_t);
		if (!(indexInfo[i].data = malloc(indexInfo[i].count * indexInfo[i].size)))
			kdo_cleanup(vk, ERRLOC, 12);

		vertexInfo[i].count	= kdo_countUniqueVertex(info[i].count, info[i].vertex);
		vertexInfo[i].size	= sizeof(Kdo_Vertex);
		if (!(vertexInfo[i].data = malloc(vertexInfo[i].count * vertexInfo[i].size)))
			kdo_cleanup(vk, ERRLOC, 12);

		indexCount = 0;
		vertexCount = 0;
		for (j = 0; j < info[i].count; j++)
		{
			for (currentIndex = 0; currentIndex < j && !kdo_vertexEqv(info[i].vertex[j], info[i].vertex[currentIndex]); currentIndex++);
			if (currentIndex < j)
				((uint32_t *) indexInfo[i].data)[indexCount++] = currentIndex;
			else
				((Kdo_Vertex *) vertexInfo[i].data)[vertexCount++] = info[i].vertex[j];
		}
	}
	kdo_addData(vk, vertexBuffer, infoCount, vertexInfo);
	kdo_addData(vk, indexBuffer, infoCount, indexInfo);

	for (i = 0; i < infoCount; i++)
	{
		free(indexInfo[i].data);
		free(vertexInfo[i].data);
	}
	free(indexInfo);
	free(vertexInfo);
}
