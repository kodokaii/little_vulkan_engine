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

#include "kdo_VkMemory.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

static void	kdo_allocBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer)
{
	VkBufferCreateInfo		bufferInfo;
	VkMemoryRequirements	memRequirements;
	VkMemoryAllocateInfo	allocInfo;

	bufferInfo.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext					= NULL;
	bufferInfo.flags					= 0;
	bufferInfo.size						= buffer->sizeUsed + buffer->sizeFree;
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
	allocInfo.memoryTypeIndex   = kdo_findMemoryType(vk, memRequirements.memoryTypeBits, buffer->properties.memoryFlags);
	if (vkAllocateMemory(vk->device.path, &allocInfo, NULL, &buffer->memory) != VK_SUCCESS)
		kdo_cleanup(vk, "Allocation buffer memory failed", 25);
	vkBindBufferMemory(vk->device.path, buffer->buffer, buffer->memory, 0);
}

static void	kdo_cpyBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferDst, Kdo_VkBuffer *bufferSrc, VkDeviceSize dstOffset, VkDeviceSize srcOffset, VkDeviceSize size)
{
	VkCommandBuffer			copy;
	VkBufferCopy			copyInfo;

	if (size)
	{
		kdo_beginUniqueCommand(vk, &copy);
		copyInfo.srcOffset	= srcOffset;
		copyInfo.dstOffset	= dstOffset;
		copyInfo.size		= size;
		vkCmdCopyBuffer(copy, bufferSrc->buffer, bufferDst->buffer, 1, &copyInfo);
		kdo_endUniqueCommand(vk, &copy);
	}
}

static void	kdo_writeHostBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer, void *data, VkDeviceSize dataSize, VkDeviceSize offset)
{
	void			*mapMemory;

	vkMapMemory(vk->device.path, buffer->memory, offset, dataSize, 0, &mapMemory);
	memcpy(mapMemory, data, dataSize);
	vkUnmapMemory(vk->device.path, buffer->memory);
}

static void	kdo_readHostBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer, void *data, VkDeviceSize dataSize, VkDeviceSize offset)
{
	void			*mapMemory;

	vkMapMemory(vk->device.path, buffer->memory, offset, dataSize, 0, &mapMemory);
	memcpy(data, mapMemory, dataSize);
	vkUnmapMemory(vk->device.path, buffer->memory);
}

Kdo_VkBuffer	kdo_newBuffer(Kdo_Vulkan *vk, VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags, Kdo_VkWait waitFlags)
{
	Kdo_VkBuffer	buffer;

	buffer.properties.usage			= usage;
	buffer.properties.memoryFlags	= memoryFlags;
	buffer.properties.waitFlags		= waitFlags;
	buffer.sizeUsed					= 0;
	buffer.sizeFree					= bufferSize;
	if (bufferSize)
		kdo_allocBuffer(vk, &buffer);

	return (buffer);
}

void	kdo_reallocBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferSrc, VkDeviceSize newSize)
{
	Kdo_VkBuffer			bufferDst = {};

	if (newSize == bufferSrc->sizeUsed + bufferSrc->sizeFree)
		return ;
	
	bufferDst.properties.usage			= bufferSrc->properties.usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bufferDst.properties.memoryFlags	= bufferSrc->properties.memoryFlags;
	bufferDst.properties.waitFlags		= bufferSrc->properties.waitFlags;
	bufferDst.sizeUsed					= kdo_minSize(bufferSrc->sizeUsed, newSize);
	bufferDst.sizeFree					= newSize - bufferDst.sizeUsed; 
	kdo_allocBuffer(vk, &bufferDst);

	kdo_cpyBuffer(vk, &bufferDst, bufferSrc, 0, 0, bufferDst.sizeUsed); 
	
	kdo_freeBuffer(vk, bufferSrc);
	*bufferSrc = bufferDst;
}

void	kdo_setData(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer, void *data, VkDeviceSize dataSize, VkDeviceSize offset)
{
	Kdo_VkBuffer	stagingBuffer;
	VkDeviceSize	bufferSize;

	if (dataSize == 0)
		return ;
	if (dataSize < 0 || buffer->sizeUsed < offset)
		kdo_cleanup(vk, "Write GPU Buffer Error", 26);

	bufferSize = kdo_maxSize(buffer->sizeUsed + buffer->sizeFree, offset + dataSize);
	kdo_reallocBuffer(vk, buffer, bufferSize);
	if (buffer->properties.memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
				kdo_writeHostBuffer(vk, buffer, data, dataSize, offset);
	else
	{
		stagingBuffer = kdo_newBuffer(vk, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);  
		kdo_writeHostBuffer(vk, &stagingBuffer, data, dataSize, 0);
		kdo_cpyBuffer(vk, buffer, &stagingBuffer, offset, 0, dataSize);
		kdo_freeBuffer(vk, &stagingBuffer);
	}
	buffer->sizeUsed = kdo_maxSize(buffer->sizeUsed, offset + dataSize);	
	buffer->sizeFree = bufferSize - buffer->sizeUsed;
}

void	kdo_getData(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer, void *data, VkDeviceSize dataSize, VkDeviceSize offset)
{
	Kdo_VkBuffer	stagingBuffer;

	if (dataSize == 0)
		return ;
	if (dataSize < 0 || (buffer->sizeUsed + buffer->sizeFree) < (offset + dataSize))
		kdo_cleanup(vk, "Read GPU Buffer Error", 27);

	if (buffer->properties.memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
				kdo_readHostBuffer(vk, buffer, data, dataSize, offset);
	else
	{
		stagingBuffer = kdo_newBuffer(vk, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);  
		kdo_cpyBuffer(vk, &stagingBuffer, buffer, 0, offset, dataSize);
		kdo_readHostBuffer(vk, &stagingBuffer, data, dataSize, offset);
		kdo_freeBuffer(vk, &stagingBuffer);
	}
}


static void	kdo_allocImageBuffer(Kdo_Vulkan *vk, Kdo_VkImageBuffer *imageBuffer)
{
	VkMemoryAllocateInfo	allocInfo;

	allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext             = NULL;
	allocInfo.allocationSize    = imageBuffer->sizeUsed + imageBuffer->sizeFree;
	allocInfo.memoryTypeIndex   = kdo_findMemoryType(vk, imageBuffer->properties.memoryFilter, imageBuffer->properties.memoryFlags);
	if (vkAllocateMemory(vk->device.path, &allocInfo, NULL, &imageBuffer->memory) != VK_SUCCESS)
		kdo_cleanup(vk, "Allocation image memory failed", 28);
}

static void	kdo_appendNewImage(Kdo_Vulkan *vk, Kdo_VkImageBuffer *imageBuffer, VkExtent3D extent, Kdo_VkImageFuncInfo funcInfo)
{
	Kdo_VkImage				newImage;
	VkImageCreateInfo		imageInfo;
	VkImageViewCreateInfo   viewInfo;
	VkMemoryRequirements    memRequirements;

	(*funcInfo.imageInfo)(extent, &imageInfo);
	vkCreateImage(vk->device.path, &imageInfo, NULL, &newImage.image);
	
	(*funcInfo.viewInfo)(newImage.image, &viewInfo);
	vkCreateImageView(vk->device.path, &viewInfo, NULL, &newImage.view);

	vkGetImageMemoryRequirements(vk->device.path, newImage.image, &memRequirements);
	newImage.extent = extent;
	newImage.size = memRequirements.size;

	kdo_reallocMerge(imageBuffer->imageCount * sizeof(Kdo_VkImage), imageBuffer->image, sizeof(Kdo_VkImage), &newImage);
}

static void	kdo_cmdImageBarrier(VkCommandBuffer commandBuffer, VkImage image, \
		VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, \
		VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, \
		VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkImageMemoryBarrier	imageBarrierInfo;

	imageBarrierInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrierInfo.pNext                           = NULL;
	imageBarrierInfo.srcAccessMask                   = srcAccessMask;
	imageBarrierInfo.dstAccessMask                   = dstAccessMask;
	imageBarrierInfo.oldLayout                       = oldLayout;
	imageBarrierInfo.newLayout                       = newLayout;
	imageBarrierInfo.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	imageBarrierInfo.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
	imageBarrierInfo.image                           = image;
	imageBarrierInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBarrierInfo.subresourceRange.baseMipLevel   = 0;
	imageBarrierInfo.subresourceRange.levelCount     = 1;
	imageBarrierInfo.subresourceRange.baseArrayLayer = 0;
	imageBarrierInfo.subresourceRange.layerCount     = 1;
	vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, NULL, 0, NULL, 1, &imageBarrierInfo);
}

Kdo_VkImageBuffer	kdo_newImageBuffer(Kdo_Vulkan *vk, VkDeviceSize bufferSize, VkImageLayout layout, VkMemoryPropertyFlags memoryFlags, uint32_t memoryFilter, Kdo_VkWait waitFlags)
{
	Kdo_VkImageBuffer		imageBuffer;

	imageBuffer.properties.layout		= layout;
	imageBuffer.properties.memoryFlags	= memoryFlags;
	imageBuffer.properties.memoryFilter	= memoryFilter;
	imageBuffer.properties.waitFlags	= waitFlags;
	imageBuffer.sizeUsed				= 0;
	imageBuffer.sizeFree				= bufferSize;
	imageBuffer.memory					= 0;
	imageBuffer.imageCount				= 0;
	imageBuffer.image					= NULL;
	if (bufferSize)
		kdo_allocImageBuffer(vk, &imageBuffer);

	return (imageBuffer);
}

void	kdo_reallocImageBuffer(Kdo_Vulkan *vk, Kdo_VkImageBuffer *imageBufferSrc, VkDeviceSize newSize, Kdo_VkImageFuncInfo funcInfo)
{
	Kdo_VkImageBuffer	imageBufferDst;

	if (newSize == imageBufferSrc->sizeUsed + imageBufferSrc->sizeFree)
		return ;
	
	imageBufferDst.properties.layout		= VK_IMAGE_LAYOUT_UNDEFINED;
	imageBufferDst.properties.memoryFlags	= imageBufferSrc->properties.memoryFlags;
	imageBufferDst.properties.memoryFilter	= imageBufferSrc->properties.memoryFilter;
	imageBufferDst.properties.waitFlags		= imageBufferSrc->properties.waitFlags;
	imageBufferDst.sizeUsed					= kdo_minSize(imageBufferSrc->sizeUsed, newSize);
	imageBufferDst.sizeFree					= newSize - imageBufferDst.sizeUsed; 
	kdo_allocImageBuffer(vk, &imageBufferDst);

	for (uint32_t i = 0; i < imageBufferSrc->imageCount && imageBufferSrc->image[i].size <= imageBufferSrc->sizeFree; i++)
		kdo_appendImageFromImage(vk, &imageBufferDst, imageBufferSrc, i, funcInfo);
	
	kdo_freeImageBuffer(vk, imageBufferSrc);
	*imageBufferSrc = imageBufferDst;
}

void	kdo_appendImageFromImage(Kdo_Vulkan *vk, Kdo_VkImageBuffer *imageBufferDst, Kdo_VkImageBuffer *imageBufferSrc, uint32_t imageIndex, Kdo_VkImageFuncInfo funcInfo)
{
	VkCommandBuffer         commandBuffer;
	VkImageCopy             copyInfo;	

	kdo_appendNewImage(vk, imageBufferDst, imageBufferSrc->image[imageIndex].extent, funcInfo);
	if (imageBufferDst->sizeFree < imageBufferDst->image[imageBufferDst->imageCount].size)
		kdo_reallocImageBuffer(vk, imageBufferDst, imageBufferDst->sizeUsed + imageBufferDst->image[imageBufferDst->imageCount].size, funcInfo);
	vkBindImageMemory(vk->device.path, imageBufferDst->image[imageBufferDst->imageCount].image, imageBufferDst->memory, imageBufferDst->sizeUsed); 

	kdo_beginUniqueCommand(vk, &commandBuffer);

	kdo_cmdImageBarrier(commandBuffer, imageBufferDst->image[imageBufferDst->imageCount].image, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	kdo_cmdImageBarrier(commandBuffer, imageBufferSrc->image[imageIndex].image, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT, imageBufferSrc->properties.layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	
	copyInfo.srcSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
	copyInfo.srcSubresource.mipLevel        = 0;
	copyInfo.srcSubresource.baseArrayLayer  = 0;
	copyInfo.srcSubresource.layerCount      = 1;
	copyInfo.srcOffset.x                    = 0;
	copyInfo.srcOffset.y                    = 0;
	copyInfo.srcOffset.z                    = 0;
	copyInfo.dstSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
	copyInfo.dstSubresource.mipLevel        = 0;
	copyInfo.dstSubresource.baseArrayLayer  = 0;
	copyInfo.dstSubresource.layerCount      = 1;
	copyInfo.dstOffset.x                    = 0;
	copyInfo.dstOffset.y                    = 0;
	copyInfo.dstOffset.z                    = 0;
	copyInfo.extent							= imageBufferSrc->image[imageIndex].extent;
	vkCmdCopyImage(commandBuffer, imageBufferSrc->image[imageIndex].image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, imageBufferDst->image[imageBufferDst->imageCount].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

	kdo_cmdImageBarrier(commandBuffer, imageBufferDst->image[imageBufferDst->imageCount].image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageBufferDst->properties.layout);
	kdo_cmdImageBarrier(commandBuffer, imageBufferSrc->image[imageIndex].image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, imageBufferSrc->properties.layout);

	kdo_endUniqueCommand(vk, &commandBuffer);

	imageBufferDst->sizeUsed += imageBufferDst->image[imageBufferDst->imageCount].size;
	imageBufferDst->sizeFree -= imageBufferDst->image[imageBufferDst->imageCount].size;
}

void	kdo_appendImageFromBuffer(Kdo_Vulkan *vk, Kdo_VkImageBuffer *imageBufferDst, Kdo_VkBuffer *bufferSrc, VkDeviceSize offset, VkExtent3D extent, Kdo_VkImageFuncInfo funcInfo)
{
	VkCommandBuffer         commandBuffer;
	VkBufferImageCopy       copyBufferInfo;

	kdo_appendNewImage(vk, imageBufferDst, extent, funcInfo);
	if (imageBufferDst->sizeFree < imageBufferDst->image[imageBufferDst->imageCount].size)
		kdo_reallocImageBuffer(vk, imageBufferDst, imageBufferDst->sizeUsed + imageBufferDst->image[imageBufferDst->imageCount].size, funcInfo);
	vkBindImageMemory(vk->device.path, imageBufferDst->image[imageBufferDst->imageCount].image, imageBufferDst->memory, imageBufferDst->sizeUsed); 

	kdo_beginUniqueCommand(vk, &commandBuffer);

	kdo_cmdImageBarrier(commandBuffer, imageBufferDst->image[imageBufferDst->imageCount].image, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	copyBufferInfo.bufferRowLength                  = 0;
	copyBufferInfo.bufferImageHeight                = 0;
	copyBufferInfo.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
	copyBufferInfo.imageSubresource.mipLevel        = 0;
	copyBufferInfo.imageSubresource.baseArrayLayer  = 0;
	copyBufferInfo.imageSubresource.layerCount      = 1;
	copyBufferInfo.imageOffset.x                    = 0;
	copyBufferInfo.imageOffset.y                    = 0;
	copyBufferInfo.imageOffset.z                    = 0;
	copyBufferInfo.bufferOffset						= offset;
	copyBufferInfo.imageExtent						= extent;
	vkCmdCopyBufferToImage(commandBuffer, bufferSrc->buffer, imageBufferDst->image[imageBufferDst->imageCount].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyBufferInfo);

	kdo_cmdImageBarrier(commandBuffer, imageBufferDst->image[imageBufferDst->imageCount].image, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, imageBufferDst->properties.layout);

	kdo_endUniqueCommand(vk, &commandBuffer);

	imageBufferDst->sizeUsed += imageBufferDst->image[imageBufferDst->imageCount].size;
	imageBufferDst->sizeFree -= imageBufferDst->image[imageBufferDst->imageCount].size;
}

void	kdo_appendImage(Kdo_Vulkan *vk, Kdo_VkImageBuffer *imageBufferDst, char *imagePath, Kdo_VkImageFuncInfo funcInfo)
{
	Kdo_VkBuffer	stagingBuffer = kdo_newBuffer(vk, 0, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);
	VkExtent3D		extent;
	int				texChannels;
	void			*data;

	extent.depth = 1;
	if (!(data      = stbi_load(imagePath, (int *)&extent.width, (int *)&extent.height, &texChannels, STBI_rgb_alpha)))
		kdo_cleanup(vk, "Image load failed", 29);

	kdo_setData(vk, &stagingBuffer, data, extent.width * extent.height * 4, 0);
	kdo_appendImageFromBuffer(vk, imageBufferDst, &stagingBuffer, 0, extent, funcInfo);
	kdo_freeBuffer(vk, &stagingBuffer);
}
