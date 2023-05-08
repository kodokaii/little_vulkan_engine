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
	return (glm_vec3_eqv_eps(vertex1.pos, vertex2.pos) && glm_vec3_eqv_eps(vertex1.color, vertex2.color) && glm_vec2_eqv_eps(vertex1.tex, vertex2.tex));
}

static uint32_t	kdo_countUniqueVertex(VkDeviceSize vertexCount, Kdo_Vertex *vertex)
{
	uint32_t	uniqueVertexCount;
	uint32_t	i;
	uint32_t	j;

	uniqueVertexCount = vertexCount;
	for (i = 0; i < vertexCount; i++)
	{
		for (j = i + 1; j < vertexCount && !kdo_vertexEqv(vertex[i], vertex[j]); j++);
		if (j < vertexCount)
			uniqueVertexCount--;
	}
	return (uniqueVertexCount);
}

static void	kdo_initTransform(Kdo_VkTransform *transform)
{
	glm_mat4_identity(transform->path);
	glm_vec3_zero(transform->pos);
	glm_vec3_one(transform->scale);
	transform->yaw		= 0.0f;
	transform->pitch	= 0.0f;
	transform->roll		= 0.0f;
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
	bufferInfo.usage					= buffer->properties.usage;
	bufferInfo.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.queueFamilyIndexCount	= 0;
	bufferInfo.pQueueFamilyIndices		= NULL;
	if (vkCreateBuffer(vk->device.path, &bufferInfo, NULL, &buffer->buffer) != VK_SUCCESS)
		kdo_cleanup(vk, "Buffer creation failed", 24);
	
	vkGetBufferMemoryRequirements(vk->device.path, buffer->buffer, &memRequirements);

	allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext             = NULL;
	allocInfo.allocationSize    = memRequirements.size;
	allocInfo.memoryTypeIndex   = kdo_findMemoryType(vk, memRequirements.memoryTypeBits, memoryFlags);
	if (vkAllocateMemory(vk->device.path, &allocInfo, NULL, &buffer->memory) != VK_SUCCESS)
		kdo_cleanup(vk, "Allocation buffer memory failed", 25);
	vkBindBufferMemory(vk->device.path, buffer->buffer, buffer->memory, 0);
}

void	kdo_allocImage(Kdo_Vulkan *vk, Kdo_VkImage *image, VkMemoryPropertyFlags memoryFlags, Kdo_VkImageInfoFunc func)
{
	VkImageCreateInfo		imageInfo;
	VkImageViewCreateInfo	viewInfo;
	VkMemoryRequirements	memRequirements;
	VkMemoryAllocateInfo	allocInfo;
	VkDeviceSize			offset;
	uint32_t				i;

	image->size	= 0;
	for(i = 0; i < image->divCount; i++)
	{
		(*func.imageInfo)(image->div[i].extent, &imageInfo);
		vkCreateImage(vk->device.path, &imageInfo, NULL, &image->div[i].image);

		vkGetImageMemoryRequirements(vk->device.path, image->div[i].image, &memRequirements);
		image->div[i].size	= memRequirements.size;
		image->size			+= memRequirements.size;
	}

	allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext             = NULL;
	allocInfo.allocationSize    = image->size;
	allocInfo.memoryTypeIndex   = kdo_findMemoryType(vk, image->properties.memoryFilter, memoryFlags);
	if (vkAllocateMemory(vk->device.path, &allocInfo, NULL, &image->memory) != VK_SUCCESS)
		kdo_cleanup(vk, "Allocation image memory failed", 26);

	offset	= 0;
	for (i = 0; i < image->divCount; i++)
	{
		vkBindImageMemory(vk->device.path, image->div[i].image, image->memory, offset);

		(*func.viewInfo)(image->div[i].image, &viewInfo);
		vkCreateImageView(vk->device.path, &viewInfo, NULL, &image->div[i].view);

		offset += image->div[i].size;
	}
}

Kdo_VkBuffer	kdo_mergeBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferSrc1, Kdo_VkBuffer *bufferSrc2)
{
	Kdo_VkBuffer			bufferDst = {};
	VkCommandBuffer			copy;
	VkBufferCopy			copyInfo;

	bufferDst.properties.usage		= bufferSrc1->properties.usage | bufferSrc2->properties.usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bufferDst.properties.waitFlags	= bufferSrc1->properties.waitFlags | bufferSrc2->properties.waitFlags;
	bufferDst.size					= bufferSrc1->size + bufferSrc2->size;
	bufferDst.divCount				= bufferSrc1->divCount + bufferSrc2->divCount;
	if (!(bufferDst.div				= kdo_mallocMerge(bufferSrc1->divCount * sizeof(Kdo_VkBufferDiv), bufferSrc1->div, bufferSrc2->divCount * sizeof(Kdo_VkBufferDiv), bufferSrc2->div)))
		kdo_cleanup(vk, ERRLOC, 12);

	for (uint32_t i = bufferSrc1->divCount; i < bufferDst.divCount; i++)
		bufferDst.div[i].offset += bufferSrc1->size;

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
	kdo_freeBuffer(vk, bufferSrc1);
	kdo_freeBuffer(vk, bufferSrc2);

	return (bufferDst);
}

Kdo_VkImage	kdo_mergeImage(Kdo_Vulkan *vk, Kdo_VkImage *imageSrc1, Kdo_VkImage *imageSrc2, Kdo_VkMergeImageInfo info)
{
	Kdo_VkImage				imageDst;
	VkCommandBuffer			copy;
	VkImageMemoryBarrier	*imageBarrierInfo;
	VkImageCopy				copyInfo;
	uint32_t				i;
	uint32_t				j;

	imageDst.properties.memoryFilter	= imageSrc1->properties.memoryFilter & imageSrc2->properties.memoryFilter;
	imageDst.properties.layout			= info.layout;
	imageDst.properties.waitFlags		= imageSrc1->properties.waitFlags | imageSrc2->properties.waitFlags;
	imageDst.divCount		= imageSrc1->divCount + imageSrc2->divCount;
	if (!(imageDst.div		= kdo_mallocMerge(imageSrc1->divCount * sizeof(Kdo_VkImageDiv), imageSrc1->div, imageSrc2->divCount * sizeof(Kdo_VkImageDiv), imageSrc2->div)))
		kdo_cleanup(vk, ERRLOC, 12);
	
	kdo_allocImage(vk, &imageDst, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, info.func);
	if (!(imageBarrierInfo	= malloc(2 * imageDst.divCount * sizeof(VkImageMemoryBarrier))))
		kdo_cleanup(vk, ERRLOC, 12);

	kdo_beginUniqueCommand(vk, &copy);
	
	for (i = 0; i < imageDst.divCount; i++)
	{
		imageBarrierInfo[i].sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrierInfo[i].pNext                           = NULL;
        imageBarrierInfo[i].srcAccessMask                   = 0;
        imageBarrierInfo[i].dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierInfo[i].oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrierInfo[i].newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierInfo[i].srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        imageBarrierInfo[i].dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        imageBarrierInfo[i].image                           = imageDst.div[i].image;
        imageBarrierInfo[i].subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBarrierInfo[i].subresourceRange.baseMipLevel   = 0;
        imageBarrierInfo[i].subresourceRange.levelCount     = 1;
        imageBarrierInfo[i].subresourceRange.baseArrayLayer = 0;
        imageBarrierInfo[i].subresourceRange.layerCount     = 1;
	}
	for (j = 0; j < imageSrc1->divCount; j++)
	{
		imageBarrierInfo[i].sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrierInfo[i].pNext                           = NULL;
        imageBarrierInfo[i].srcAccessMask                   = 0;
        imageBarrierInfo[i].dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierInfo[i].oldLayout                       = imageSrc1->properties.layout;
        imageBarrierInfo[i].newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageBarrierInfo[i].srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        imageBarrierInfo[i].dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        imageBarrierInfo[i].image                           = imageSrc1->div[j].image;
        imageBarrierInfo[i].subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBarrierInfo[i].subresourceRange.baseMipLevel   = 0;
        imageBarrierInfo[i].subresourceRange.levelCount     = 1;
        imageBarrierInfo[i].subresourceRange.baseArrayLayer = 0;
        imageBarrierInfo[i].subresourceRange.layerCount     = 1;

		i++;
	}
	for (j = 0; j < imageSrc2->divCount; j++)
	{
		imageBarrierInfo[i].sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrierInfo[i].pNext                           = NULL;
        imageBarrierInfo[i].srcAccessMask                   = 0;
        imageBarrierInfo[i].dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierInfo[i].oldLayout                       = imageSrc2->properties.layout;
        imageBarrierInfo[i].newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageBarrierInfo[i].srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        imageBarrierInfo[i].dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        imageBarrierInfo[i].image                           = imageSrc2->div[j].image;
        imageBarrierInfo[i].subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBarrierInfo[i].subresourceRange.baseMipLevel   = 0;
        imageBarrierInfo[i].subresourceRange.levelCount     = 1;
        imageBarrierInfo[i].subresourceRange.baseArrayLayer = 0;
        imageBarrierInfo[i].subresourceRange.layerCount     = 1;
		i++;
	}
	vkCmdPipelineBarrier(copy, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 2 * imageDst.divCount, imageBarrierInfo);

	copyInfo.srcSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	copyInfo.srcSubresource.mipLevel		= 0;
	copyInfo.srcSubresource.baseArrayLayer	= 0;
	copyInfo.srcSubresource.layerCount		= 1;
	copyInfo.srcOffset.x					= 0;
	copyInfo.srcOffset.y					= 0;
	copyInfo.srcOffset.z					= 0;
	copyInfo.dstSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	copyInfo.dstSubresource.mipLevel		= 0;
	copyInfo.dstSubresource.baseArrayLayer	= 0;
	copyInfo.dstSubresource.layerCount		= 1;
	copyInfo.dstOffset.x					= 0;
	copyInfo.dstOffset.y					= 0;
	copyInfo.dstOffset.z					= 0;

	for (i = 0; i < imageSrc1->divCount; i++)
    {
		copyInfo.extent	= imageSrc1->div[i].extent;
		vkCmdCopyImage(copy, imageSrc1->div[i].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, imageDst.div[i].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

		imageBarrierInfo[i].srcAccessMask   = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierInfo[i].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
        imageBarrierInfo[i].oldLayout       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrierInfo[i].newLayout       = imageDst.properties.layout;

	}
	for (j = 0; j < imageSrc2->divCount; j++)
    {
		copyInfo.extent	= imageSrc2->div[j].extent;
		vkCmdCopyImage(copy, imageSrc2->div[j].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, imageDst.div[i].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

		imageBarrierInfo[i].srcAccessMask   = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierInfo[i].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
        imageBarrierInfo[i].oldLayout       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrierInfo[i].newLayout       = imageDst.properties.layout;
		i++;
	}
	vkCmdPipelineBarrier(copy, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, imageDst.divCount, imageBarrierInfo);

	kdo_endUniqueCommand(vk, &copy);
	free(imageBarrierInfo);
	kdo_freeImage(vk, imageSrc1);
	kdo_freeImage(vk, imageSrc2);

	return (imageDst);
}

Kdo_VkImage	kdo_mergeBufferToImage(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferSrc, Kdo_VkImage *imageSrc, Kdo_VkMergeBufferToImageInfo info)
{
	Kdo_VkImage				imageDst;
	VkCommandBuffer			copy;
	VkImageMemoryBarrier	*imageBarrierInfo;
	VkImageCopy				copyImageInfo;
	VkBufferImageCopy		copyBufferInfo;
	uint32_t				i;
	uint32_t				j;

	imageDst.properties.memoryFilter	= imageSrc->properties.memoryFilter;
	imageDst.properties.layout			= info.layout;
	imageDst.properties.waitFlags		= imageSrc->properties.waitFlags;
	imageDst.divCount					= imageSrc->divCount + info.imagesCount;
	if (!(imageDst.div					= malloc(imageDst.divCount * sizeof(Kdo_VkImageDiv))))
		kdo_cleanup(vk, ERRLOC, 12);

	memcpy(imageDst.div, imageSrc->div, imageSrc->divCount * sizeof(Kdo_VkImageDiv));
	for (i = 0; i < info.imagesCount; i++)
		imageDst.div[imageSrc->divCount + i].extent = info.extents[i];

	kdo_allocImage(vk, &imageDst, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, info.func);
	if (!(imageBarrierInfo	= malloc((imageDst.divCount + imageSrc->divCount) * sizeof(VkImageMemoryBarrier))))
		kdo_cleanup(vk, ERRLOC, 12);

	kdo_beginUniqueCommand(vk, &copy);

	for (i = 0; i < imageDst.divCount; i++)
	{
		imageBarrierInfo[i].sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrierInfo[i].pNext                           = NULL;
        imageBarrierInfo[i].srcAccessMask                   = 0;
        imageBarrierInfo[i].dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierInfo[i].oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
        imageBarrierInfo[i].newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageBarrierInfo[i].srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        imageBarrierInfo[i].dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        imageBarrierInfo[i].image                           = imageDst.div[i].image;
        imageBarrierInfo[i].subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBarrierInfo[i].subresourceRange.baseMipLevel   = 0;
        imageBarrierInfo[i].subresourceRange.levelCount     = 1;
        imageBarrierInfo[i].subresourceRange.baseArrayLayer = 0;
        imageBarrierInfo[i].subresourceRange.layerCount     = 1;
	}
	for (j = 0; j < imageSrc->divCount; j++)
	{
		imageBarrierInfo[i].sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageBarrierInfo[i].pNext                           = NULL;
        imageBarrierInfo[i].srcAccessMask                   = 0;
        imageBarrierInfo[i].dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierInfo[i].oldLayout                       = imageSrc->properties.layout;
        imageBarrierInfo[i].newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageBarrierInfo[i].srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        imageBarrierInfo[i].dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        imageBarrierInfo[i].image                           = imageSrc->div[j].image;
        imageBarrierInfo[i].subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBarrierInfo[i].subresourceRange.baseMipLevel   = 0;
        imageBarrierInfo[i].subresourceRange.levelCount     = 1;
        imageBarrierInfo[i].subresourceRange.baseArrayLayer = 0;
        imageBarrierInfo[i].subresourceRange.layerCount     = 1;
		i++;
	}
	vkCmdPipelineBarrier(copy, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, imageDst.divCount + imageSrc->divCount, imageBarrierInfo);

	copyImageInfo.srcSubresource.aspectMask			= VK_IMAGE_ASPECT_COLOR_BIT;
	copyImageInfo.srcSubresource.mipLevel			= 0;
	copyImageInfo.srcSubresource.baseArrayLayer		= 0;
	copyImageInfo.srcSubresource.layerCount			= 1;
	copyImageInfo.srcOffset.x						= 0;
	copyImageInfo.srcOffset.y						= 0;
	copyImageInfo.srcOffset.z						= 0;
	copyImageInfo.dstSubresource.aspectMask			= VK_IMAGE_ASPECT_COLOR_BIT;
	copyImageInfo.dstSubresource.mipLevel			= 0;
	copyImageInfo.dstSubresource.baseArrayLayer		= 0;
	copyImageInfo.dstSubresource.layerCount			= 1;
	copyImageInfo.dstOffset.x						= 0;
	copyImageInfo.dstOffset.y						= 0;
	copyImageInfo.dstOffset.z						= 0;

	copyBufferInfo.bufferRowLength					= 0;
    copyBufferInfo.bufferImageHeight				= 0;
    copyBufferInfo.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
    copyBufferInfo.imageSubresource.mipLevel        = 0;
    copyBufferInfo.imageSubresource.baseArrayLayer  = 0;
    copyBufferInfo.imageSubresource.layerCount      = 1;
    copyBufferInfo.imageOffset.x                    = 0;
    copyBufferInfo.imageOffset.y                    = 0;
    copyBufferInfo.imageOffset.z                    = 0;

	for (i = 0; i < imageSrc->divCount; i++)
	{
		copyImageInfo.extent	= imageSrc->div[i].extent;
		vkCmdCopyImage(copy, imageSrc->div[i].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, imageDst.div[i].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyImageInfo);

		imageBarrierInfo[i].srcAccessMask	= VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierInfo[i].dstAccessMask	= VK_ACCESS_SHADER_READ_BIT;
        imageBarrierInfo[i].oldLayout		= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrierInfo[i].newLayout		= imageDst.properties.layout;
	}

	for (j = 0; j < bufferSrc->divCount; j++)
	{
		copyBufferInfo.bufferOffset	= bufferSrc->div[j].offset;
        copyBufferInfo.imageExtent	= imageDst.div[i].extent;
        vkCmdCopyBufferToImage(copy, bufferSrc->buffer, imageDst.div[i].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyBufferInfo);

		imageBarrierInfo[i].srcAccessMask	= VK_ACCESS_TRANSFER_WRITE_BIT;
        imageBarrierInfo[i].dstAccessMask	= VK_ACCESS_SHADER_READ_BIT;
        imageBarrierInfo[i].oldLayout		= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrierInfo[i].newLayout		= imageDst.properties.layout;
		i++;
	}
	vkCmdPipelineBarrier(copy, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, imageDst.divCount, imageBarrierInfo);

	kdo_endUniqueCommand(vk, &copy);
	free(imageBarrierInfo);
	kdo_freeBuffer(vk, bufferSrc);
	kdo_freeImage(vk, imageSrc);

	return (imageDst);
}

Kdo_VkBuffer	kdo_loadData(Kdo_Vulkan *vk, uint32_t infoCount, Kdo_VkLoadDataInfo *info)
{
	Kdo_VkBuffer	bufferDst = {};
	VkDeviceSize	offset;
	uint32_t		i;
	void			*data;

	bufferDst.properties.usage		= VK_BUFFER_USAGE_TRANSFER_SRC_BIT; 
	bufferDst.size					= 0;
	bufferDst.divCount				= infoCount;
	if (!(bufferDst.div				= malloc(infoCount * sizeof(Kdo_VkBufferDiv))))
		kdo_cleanup(vk, ERRLOC, 12);

	for (i = 0; i < infoCount; i++)
	{
		bufferDst.div[i].offset			= bufferDst.size;
		bufferDst.div[i].elementSize	= info[i].elementSize;
		bufferDst.div[i].count			= info[i].count;

		bufferDst.size += info[i].count * info[i].elementSize;
	}

	kdo_allocBuffer(vk, &bufferDst, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	offset = 0;
	vkMapMemory(vk->device.path, bufferDst.memory, 0, VK_WHOLE_SIZE, 0, &data);
	for (i = 0; i < infoCount; i++)
	{
		memcpy(data + offset, info[i].data, info[i].count * info[i].elementSize);
		offset += info[i].count * info[i].elementSize;
	}
	vkUnmapMemory(vk->device.path, bufferDst.memory);
	
	return (bufferDst);
}

void	kdo_loadMesh(Kdo_Vulkan *vk, Kdo_VkBuffer *vertexBuffer, Kdo_VkBuffer *indexBuffer, uint32_t infoCount, Kdo_VkLoadMeshInfo *info)
{
	Kdo_VkBuffer		stagingBuffer;
	Kdo_VkLoadDataInfo	*vertexInfo;
	Kdo_VkLoadDataInfo	*indexInfo;
	uint32_t			currentIndex;
	uint32_t			vertexCount;
	uint32_t			i;
	uint32_t			j;

	if (!(vertexInfo = malloc(infoCount * sizeof(Kdo_VkLoadDataInfo))))
		kdo_cleanup(vk, ERRLOC, 12);	
	if (!(indexInfo = malloc(infoCount * sizeof(Kdo_VkLoadDataInfo))))
		kdo_cleanup(vk, ERRLOC, 12);	

	for (i = 0; i < infoCount; i++)
	{
		indexInfo[i].count			= info[i].count;
		indexInfo[i].elementSize	= sizeof(uint32_t);
		if (!(indexInfo[i].data = malloc(indexInfo[i].count * indexInfo[i].elementSize)))
			kdo_cleanup(vk, ERRLOC, 12);

		vertexInfo[i].count			= kdo_countUniqueVertex(info[i].count, info[i].vertex);
		vertexInfo[i].elementSize	= sizeof(Kdo_Vertex);
		if (!(vertexInfo[i].data = malloc(vertexInfo[i].count * vertexInfo[i].elementSize)))
			kdo_cleanup(vk, ERRLOC, 12);

		vertexCount = 0;
		for (j = 0; j < info[i].count; j++)
		{
			for (currentIndex = 0; currentIndex < j && !kdo_vertexEqv(info[i].vertex[j], info[i].vertex[currentIndex]); currentIndex++);
			if (currentIndex == j)
			{
				((Kdo_Vertex *) vertexInfo[i].data)[vertexCount]	= info[i].vertex[j];
				((uint32_t *) indexInfo[i].data)[j]					= vertexCount;
				vertexCount++;
			}
			else
				((uint32_t *) indexInfo[i].data)[j]					= currentIndex;
		}
	}

	stagingBuffer	= kdo_loadData(vk, infoCount, vertexInfo);
	*vertexBuffer	= kdo_mergeBuffer(vk, vertexBuffer, &stagingBuffer);

	stagingBuffer	= kdo_loadData(vk, infoCount, indexInfo);
	*indexBuffer	= kdo_mergeBuffer(vk, indexBuffer, &stagingBuffer);

	for (i = 0; i < infoCount; i++)
	{
		free(indexInfo[i].data);
		free(vertexInfo[i].data);
	}
	free(indexInfo);
	free(vertexInfo);
}

void	kdo_loadTextures(Kdo_Vulkan *vk, Kdo_VkImage *textures, uint32_t texturesCount, char **texturesPath)
{
	Kdo_VkBuffer					stagingBuffer;
	Kdo_VkLoadDataInfo				*loadInfo;
	Kdo_VkMergeBufferToImageInfo	mergeInfo;
	int								texWidth;
	int								texHeight;
	int								texChannels;
	uint32_t						i;

	if (!(loadInfo		= malloc(texturesCount * sizeof(Kdo_VkLoadDataInfo))))
		kdo_cleanup(vk, ERRLOC, 12);

	mergeInfo.layout			= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	mergeInfo.func.imageInfo	= kdo_imageTextureInfo;
	mergeInfo.func.viewInfo		= kdo_viewTextureInfo;
	mergeInfo.imagesCount		= texturesCount;
	if (!(mergeInfo.extents		= malloc(texturesCount * sizeof(Kdo_VkImageDiv))))
		kdo_cleanup(vk, ERRLOC, 12);

	for (i = 0; i < texturesCount; i++)
	{
		if (!(loadInfo[i].data		= stbi_load(texturesPath[i], &texWidth, &texHeight, &texChannels, STBI_rgb_alpha)))
			kdo_cleanup(vk, "Textures load failed", 27);

		loadInfo[i].count		= texWidth * texHeight;
		loadInfo[i].elementSize	= 4;

		mergeInfo.extents[i].width	= texWidth;
		mergeInfo.extents[i].height	= texHeight;
		mergeInfo.extents[i].depth	= 1;
	}

	stagingBuffer		= kdo_loadData(vk, texturesCount, loadInfo);
	*textures			= kdo_mergeBufferToImage(vk, &stagingBuffer, textures, mergeInfo);

	free(loadInfo);
	free(mergeInfo.extents);
}

void	kdo_loadObject(Kdo_Vulkan *vk, Kdo_VkObject *object, uint32_t infoCount, Kdo_VkLoadObjectInfo *info)
{
	VkDescriptorSetAllocateInfo     allocInfo;
	Kdo_VkObjectDiv					**current;
	Kdo_VkLoadMeshInfo				*meshInfo;
	char							**texturesPath;
	uint32_t						i;

	allocInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext					= NULL;
	allocInfo.descriptorPool		= vk->core.descriptorPool;
	allocInfo.descriptorSetCount	= 1;
	allocInfo.pSetLayouts			= &vk->graphicsPipeline.descriptorLayout;

	if (!(meshInfo = malloc(infoCount * sizeof(Kdo_VkLoadMeshInfo))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(texturesPath = malloc(infoCount * sizeof(char *))))
		kdo_cleanup(vk, ERRLOC, 12);

	current = &object->div;
	while (*current)
		current = &(*current)->next;

	for (i = 0; i < infoCount; i++)
	{
		if (!(*current = malloc(sizeof(Kdo_VkObjectDiv))))
			kdo_cleanup(vk, ERRLOC, 12);

		(*current)->name			= info[i].name;
		(*current)->count			= info[i].objectsCount;
		(*current)->status			= info[i].status;
		(*current)->vertexIndex		= object->vertex->divCount + i;
		(*current)->indexIndex		= object->index->divCount + i;
		(*current)->textureIndex	= object->images->divCount + i;
		(*current)->sampler			= info[i].sampler;
		(*current)->next			= NULL;

		if (!((*current)->model		= malloc(info[i].objectsCount * sizeof(Kdo_VkTransform))))
			kdo_cleanup(vk, ERRLOC, 12);

		for (uint32_t j = 0; j < info[i].objectsCount; j++)
			kdo_initTransform((*current)->model + j);
	
		if (vkAllocateDescriptorSets(vk->device.path, &allocInfo, &(*current)->descripteurSet) != VK_SUCCESS)
			kdo_cleanup(vk, "Descriptor set allocation failed", 35);

		meshInfo[i].count	= info[i].vertexCount;
		meshInfo[i].vertex	= info[i].vertex;
		texturesPath[i]		= info[i].texturePath;

		current = &(*current)->next;
	}
	kdo_loadMesh(vk, object->vertex, object->index, infoCount, meshInfo);
	kdo_loadTextures(vk, object->images, infoCount, texturesPath);
	object->divCount	+= infoCount;

	free(meshInfo);
	free(texturesPath);
}

void	kdo_updateDescripteur(Kdo_Vulkan *vk, Kdo_VkObject *object)
{
	Kdo_VkObjectDiv			**current;
	VkDescriptorImageInfo	imageInfo;
	VkWriteDescriptorSet	descriptorWrite;

	current = &object->div;
	while (*current)
	{
		imageInfo.sampler		= *(*current)->sampler;
		imageInfo.imageView		= object->images->div[(*current)->textureIndex].view;
		imageInfo.imageLayout	= object->images->properties.layout;;

		descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.pNext				= NULL;
		descriptorWrite.dstSet				= (*current)->descripteurSet;
		descriptorWrite.dstBinding			= 0;
		descriptorWrite.dstArrayElement		= 0;
		descriptorWrite.descriptorCount		= 1;
		descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.pImageInfo			= &imageInfo;
		descriptorWrite.pBufferInfo			= NULL;
		descriptorWrite.pTexelBufferView	= NULL;
		vkUpdateDescriptorSets(vk->device.path, 1, &descriptorWrite, 0, NULL);

		current = &(*current)->next;
	}
}

Kdo_VkObjectDiv	*kdo_getObject(Kdo_VkObject *object, uint32_t index)
{
	Kdo_VkObjectDiv	*current;

	current = object->div;
	if (current)
	{
		while (index-- && current->next)
			current = current->next;
	}
	return (current);
}

void	kdo_changeObjectCount(Kdo_Vulkan *vk, Kdo_VkObject *object, uint32_t index, uint32_t count)
{
	Kdo_VkObjectDiv	*current;
	Kdo_VkTransform	*model;

	current = kdo_getObject(object, index);
	
	if (!(model = malloc(count * sizeof(Kdo_VkTransform))))
		kdo_cleanup(vk, ERRLOC, 12);

	if (current->count < count)
	{
		memcpy(model, current->model, current->count * sizeof(Kdo_VkTransform));
		for (uint32_t i = current->count; i < count; i++)
			kdo_initTransform(model + i);
	}
	else
		memcpy(model, current->model, count * sizeof(Kdo_VkTransform));
	
	free(current->model);
	current->model = model;
	current->count = count;
}

void	kdo_updateModel(Kdo_VkTransform *model, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		glm_scale_make(model[i].path, model[i].scale);
		glm_translate(model[i].path, model[i].pos);
		glm_rotate(model[i].path, model[i].yaw, GLM_ZUP);
		glm_rotate(model[i].path, model[i].pitch, GLM_XUP); 
		glm_rotate(model[i].path, model[i].roll, GLM_YUP);
	}
}
