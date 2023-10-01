/* ***********************************************************************************
 * */
/*       :::    ::: ::::::::  :::::::::   ::::::::  :::    :::     :::
 *       ::::::::::: */
/*      :+:   :+: :+:    :+: :+:    :+: :+:    :+: :+:   :+:    :+: :+:
 *      :+:      */
/*     +:+  +:+  +:+    +:+ +:+    +:+ +:+    +:+ +:+  +:+    +:+   +:+
 *     +:+       */
/*    +#++:++   +#+    +:+ +#+    +:+ +#+    +:+ +#++:++    +#++:++#++:     +#+
 *    */
/*   +#+  +#+  +#+    +#+ +#+    +#+ +#+    +#+ +#+  +#+   +#+     +#+     +#+
 *   */
/*  #+#   #+# #+#    #+# #+#    #+# #+#    #+# #+#   #+#  #+#     #+#     #+#
 *  */
/* ###    ### ########  #########   ########  ###    ### ###     ###
 * ###########       */
/*                        <kodokai.featheur@gmail.com>
 *                        */
/* ***********************************************************************************
 * */

#include "kdo_VkMemory.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

static void	kdo_allocImageBuffer(Kdo_Vulkan *vk, Kdo_VkGPUImageBuffer *imageBuffer)
{
	VkMemoryAllocateInfo	allocInfo;

	allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext             = NULL;
	allocInfo.allocationSize    = imageBuffer->sizeUsed + imageBuffer->sizeFree;
	allocInfo.memoryTypeIndex   = kdo_findMemoryType(vk, imageBuffer->properties.memoryFilter, imageBuffer->properties.memoryFlags);
	if (vkAllocateMemory(vk->device.path, &allocInfo, NULL, &imageBuffer->memory) != VK_SUCCESS)
		kdo_cleanup(vk, "Allocation image memory failed", 27);
}

static void	kdo_appendNewImage(Kdo_Vulkan *vk, Kdo_VkGPUImageBuffer *imageBuffer, VkExtent3D extent, Kdo_VkImageFuncInfo funcInfo)
{
	VkImage					newImage;
	VkImageView				newView;
	VkImageCreateInfo		imageInfo;
	VkImageViewCreateInfo   viewInfo;
	VkMemoryRequirements    memRequirements;
	VkDeviceSize			alignment;

	(*funcInfo.imageInfo)(extent, &imageInfo);
	vkCreateImage(vk->device.path, &imageInfo, NULL, &newImage);

	vkGetImageMemoryRequirements(vk->device.path, newImage, &memRequirements);
	alignment				= imageBuffer->sizeUsed % memRequirements.alignment;

	if (imageBuffer->sizeFree < alignment + memRequirements.size)
		kdo_reallocImageBuffer(vk, imageBuffer, imageBuffer->sizeUsed + alignment + memRequirements.size, funcInfo);
	vkBindImageMemory(vk->device.path, newImage, imageBuffer->memory, imageBuffer->sizeUsed + alignment); 
	
	(*funcInfo.viewInfo)(newImage, &viewInfo);
	vkCreateImageView(vk->device.path, &viewInfo, NULL, &newView);

	if (!(imageBuffer->image = realloc(imageBuffer->image, (imageBuffer->imageCount + 1) * sizeof(Kdo_VkImage))))
		kdo_cleanup(vk, ERRLOC, 12);
	imageBuffer->image[imageBuffer->imageCount].image	= newImage;
	imageBuffer->image[imageBuffer->imageCount].view	= newView;
	imageBuffer->image[imageBuffer->imageCount].extent	= extent;
	imageBuffer->image[imageBuffer->imageCount].size	= memRequirements.size;

	imageBuffer->sizeUsed	+= alignment;
	imageBuffer->sizeFree	-= alignment;
}

Kdo_VkGPUImageBuffer	kdo_newImageBuffer(Kdo_Vulkan *vk, VkDeviceSize bufferSize, VkImageLayout layout, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t memoryFilter, Kdo_VkWait waitFlags)
{
	Kdo_VkGPUImageBuffer		imageBuffer;

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

void	kdo_reallocImageBuffer(Kdo_Vulkan *vk, Kdo_VkGPUImageBuffer *imageBufferSrc, VkDeviceSize newSize, Kdo_VkImageFuncInfo funcInfo)
{
	Kdo_VkGPUImageBuffer	imageBufferDst;

	if (newSize == imageBufferSrc->sizeUsed + imageBufferSrc->sizeFree)
		return ;
	
	imageBufferDst.properties.layout		= imageBufferSrc->properties.layout;
	imageBufferDst.properties.memoryFlags	= imageBufferSrc->properties.memoryFlags;
	imageBufferDst.properties.memoryFilter	= imageBufferSrc->properties.memoryFilter;
	imageBufferDst.properties.waitFlags		= imageBufferSrc->properties.waitFlags;
	imageBufferDst.sizeUsed					= 0;
	imageBufferDst.sizeFree					= newSize; 
	imageBufferDst.imageCount				= 0;
	imageBufferDst.image					= NULL;
	kdo_allocImageBuffer(vk, &imageBufferDst);

	for (uint32_t i = 0; i < imageBufferSrc->imageCount; i++)
		kdo_appendImageFromImage(vk, &imageBufferDst, imageBufferSrc, i, funcInfo);
	
	kdo_freeImageBuffer(vk, imageBufferSrc);
	*imageBufferSrc = imageBufferDst;
}

void	kdo_appendImageFromImage(Kdo_Vulkan *vk, Kdo_VkGPUImageBuffer *imageBufferDst, Kdo_VkGPUImageBuffer *imageBufferSrc, uint32_t imageIndex, Kdo_VkImageFuncInfo funcInfo)
{
	VkCommandBuffer         commandBuffer;
	VkImageCopy             copyInfo;	

	kdo_appendNewImage(vk, imageBufferDst, imageBufferSrc->image[imageIndex].extent, funcInfo);

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
	imageBufferDst->imageCount++;
}

void	kdo_appendImageFromBuffer(Kdo_Vulkan *vk, Kdo_VkGPUImageBuffer *imageBufferDst, Kdo_VkGPUBuffer *bufferSrc, VkDeviceSize offset, VkExtent3D extent, Kdo_VkImageFuncInfo funcInfo)
{
	VkCommandBuffer         commandBuffer;
	VkBufferImageCopy       copyBufferInfo;

	kdo_appendNewImage(vk, imageBufferDst, extent, funcInfo);

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
	imageBufferDst->imageCount++;
}

uint32_t	kdo_appendImage(Kdo_Vulkan *vk, Kdo_VkGPUImageBuffer *imageBufferDst, char *imagePath, Kdo_VkImageFuncInfo funcInfo)
{
	Kdo_VkGPUBuffer	stagingBuffer = kdo_newBuffer(vk, 0, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0);
	VkExtent3D		extent;
	int				texChannels;
	void			*data;

	extent.depth = 1;
	if (!(data      = stbi_load(imagePath, (int *)&extent.width, (int *)&extent.height, &texChannels, STBI_rgb_alpha)))
		return (1);

	kdo_setData(vk, &stagingBuffer, data, extent.width * extent.height * 4, 0);
	kdo_appendImageFromBuffer(vk, imageBufferDst, &stagingBuffer, 0, extent, funcInfo);
	kdo_freeBuffer(vk, &stagingBuffer);

	return (0);


}

VkResult		kdo_newGPUMemory(VkDevice device, uint32_t memoryFilter, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, Kdo_VkWait waitFlags, VkDeviceSize size, Kdo_VkGPUMemory *memory)
{
	VkMemoryAllocateInfo	allocInfo; 
	VkResult				returnCode;

	memory->path		= VK_NULL_HANDLE;
	memory->waitFlags	= waitFlags;
	memory->size		= size;
	if (size == 0)
		return (VK_SUCCESS);

	allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext             = NULL;
	allocInfo.allocationSize    = size;
	KDO_VK_ASSERT(kdo_findMemoryType(memoryFilter, memoryFlags, memoryProperties, &allocInfo.memoryTypeIndex));
	KDO_VK_ASSERT(vkAllocateMemory(device, &allocInfo, NULL, &memory->path));

	return (VK_SUCCESS);
}

VkResult		kdo_newGPUBuffer(VkDevice device, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, Kdo_VkWait waitFlags, VkBufferUsageFlags usage, VkDeviceSize size, Kdo_VkGPUBuffer *buffer)
{
	VkBufferCreateInfo		bufferInfo;
	VkMemoryRequirements	memoryRequirements;
	VkResult				returnCode;

	buffer->path		= VK_NULL_HANDLE;
	buffer->usage		= usage;
	buffer->fillSize	= 0;
	if (size == 0)
		return (kdo_newGPUMemory(device, 0, memoryFlags, memoryProperties, waitFlags, 0, &buffer->memory));

	bufferInfo.sType					= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext					= NULL;
	bufferInfo.flags					= 0;
	bufferInfo.size						= size;
	bufferInfo.usage					= usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bufferInfo.sharingMode				= VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.queueFamilyIndexCount	= 0;
	bufferInfo.pQueueFamilyIndices		= NULL;
	KDO_VK_ASSERT(vkCreateBuffer(device, &bufferInfo, NULL, &buffer->path) != VK_SUCCESS);
	vkGetBufferMemoryRequirements(device, buffer->path, &memoryRequirements);

	KDO_VK_ASSERT(kdo_newGPUMemory(device, memoryRequirements.memoryTypeBits, memoryFlags, memoryProperties, waitFlags, memoryRequirements.size, &buffer->memory));
	KDO_VK_ASSERT(vkBindBufferMemory(device, buffer->path, buffer->memory.path, 0));

	return (VK_SUCCESS);
}

VkResult		kdo_newGPUStagingBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, VkDeviceSize dataSize, void *data, Kdo_VkGPUBuffer *stagingBuffer)
{
	VkResult	returnCode;

	KDO_VK_ASSERT(kdo_newGPUBuffer(device, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memoryProperties, WAIT_NONE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, dataSize, stagingBuffer));
	KDO_VK_ASSERT(kdo_writeHostGPUMemory(&stagingBuffer->memory, device, 0, dataSize, data));
	stagingBuffer->fillSize	= dataSize;
	
	return (VK_SUCCESS);
}

VkResult		kdo_newGPUImage(VkDevice device, VkExtent3D extent, void (*newImageInfo)(VkExtent3D, VkImageCreateInfo*), void (*newViewInfo)(VkImage, VkImageViewCreateInfo*), VkDeviceSize offset, Kdo_VkGPUMemory *memory, Kdo_VkGPUImage *image)
{
	VkImageCreateInfo       imageInfo;
	VkImageViewCreateInfo   viewInfo;
	VkMemoryRequirements    memRequirements;
	VkResult				returnCode;

	image->path		= VK_NULL_HANDLE;
	image->view		= VK_NULL_HANDLE;
	image->extent	= extent;
	image->offset	= offset;

	if (extent.width == 0 || extent.height == 0 || extent.depth == 0)
		return (VK_SUCCESS);

	(*newImageInfo)(extent, &imageInfo);
	KDO_VK_ASSERT(vkCreateImage(device, &imageInfo, NULL, &image->path));

	vkGetImageMemoryRequirements(device, image->path, &memRequirements);
	offset += offset % memRequirements.alignment;
	if (memory->size < offset + memRequirements.size)
		return (VK_ERROR_MEMORY_MAP_FAILED);

	vkBindImageMemory(device, image->path, memory->path, offset);

	(*newViewInfo)(image->path, &viewInfo);
	KDO_VK_ASSERT(vkCreateImageView(device, &viewInfo, NULL, &image->view));


	return (VK_SUCCESS);
}

VkResult		kdo_newGPUImageBuffer(VkDevice device, uint32_t memoryFilter, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, VkImageLayout layout, Kdo_VkWait waitFlags, VkDeviceSize size, Kdo_VkGPUImageBuffer *imageBuffer)
{
	VkResult				returnCode;

	imageBuffer->imageCount		= 0;
	imageBuffer->imageArray		= NULL;
	imageBuffer->layout			= layout;
	imageBuffer->fillSize		= 0;
	imageBuffer->memoryFilter	= memoryFilter;

	KDO_VK_ASSERT(kdo_newGPUMemory(device, memoryFilter, memoryFlags, memoryProperties, waitFlags, size, &imageBuffer->memory));

	return (VK_SUCCESS);
}

VkResult		kdo_newCPUBuffer(VkDeviceSize size, Kdo_VkCPUBuffer *buffer)
{
	if (!(buffer->path	= malloc(size)))
		return (VK_ERROR_OUT_OF_HOST_MEMORY);
	buffer->size		= size;
	buffer->fillSize	= 0;

	return (VK_SUCCESS);
}

VkResult		kdo_newBSTNode(uint32_t index, Kdo_VkBSTNode **node)
{
	if (!(*node = malloc(sizeof(Kdo_VkBSTNode))))
		return (VK_ERROR_OUT_OF_HOST_MEMORY);
	(*node)->left	= NULL;
	(*node)->right	= NULL;
	(*node)->index	= index;

	return (VK_SUCCESS);
}

VkResult		kdo_newBST(VkDeviceSize elementSize, void **bufferPointer, Kdo_VkBST *bst)
{
	bst->node			= NULL;
	bst->bufferPointer	= bufferPointer;
	bst->elementSize	= elementSize;
	bst->endIndex		= 0;

	return (VK_SUCCESS);
}

VkResult		kdo_newBuffer(VkDevice device, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, Kdo_VkWait waitFlags, VkBufferUsageFlags usage, VkDeviceSize size, Kdo_VkBuffer *buffer)
{
	VkResult	returnCode;

	KDO_VK_ASSERT(kdo_newGPUBuffer(device, memoryFlags, memoryProperties, waitFlags, usage, size, &buffer->GPU));
	KDO_VK_ASSERT(kdo_newCPUBuffer(size, &buffer->CPU));

	return (VK_SUCCESS);
}

VkResult		kdo_newSetBuffer(VkDevice device, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, Kdo_VkWait waitFlags, VkBufferUsageFlags usage, VkDeviceSize elementSize, VkDeviceSize size, Kdo_VkSetBuffer *buffer)
{
	KDO_VK_ASSERT(kdo_newGPUBuffer(device, memoryFlags, memoryProperties, waitFlags, usage, size, &buffer->GPU));
	KDO_VK_ASSERT(kdo_newCPUBuffer(size, &buffer->CPU));
	KDO_VK_ASSERT(kdo_newBST(elementSize, &buffer->CPU.path, &buffer->BST));

	return (VK_SUCCESS);
}

VkResult		kdo_newSetImageBuffer(VkDevice device, uint32_t memoryFilter, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, VkImageLayout layout, Kdo_VkWait waitFlags, VkDeviceSize size, Kdo_VkSetImageBuffer *imageBuffer)
{

	KDO_VK_ASSERT(kdo_newGPUImageBuffer(device, memoryFilter, memoryFlags, memoryProperties, layout, waitFlags, size, &imageBuffer->GPU));
	KDO_VK_ASSERT(kdo_newCPUBuffer(size, &imageBuffer->CPU));
	KDO_VK_ASSERT(kdo_newBST(sizeof(char *), &imageBuffer->CPU.path, &imageBuffer->BST));

	return (VK_SUCCESS);
}

void		kdo_freeGPUMemory(Kdo_VkGPUMemory *memory, VkDevice device)
{
	KDO_VK_FREE(vkFreeMemory, device, memory->path);
	memory->size	= 0;
}

void		kdo_freeGPUBuffer(Kdo_VkGPUBuffer *buffer, VkDevice device)
{
	KDO_VK_FREE(vkDestroyBuffer, device, buffer->path);
	kdo_freeGPUMemory(&buffer->memory, device);
	buffer->fillSize	= 0;
}

void		kdo_freeGPUImage(Kdo_VkGPUImage *image, VkDevice device)
{
	KDO_VK_FREE(vkDestroyImageView, device, image->view);
	KDO_VK_FREE(vkDestroyImage, device, image->path);
}

void		kdo_freeGPUImageBuffer(Kdo_VkGPUImageBuffer *imageBuffer, VkDevice device)
{
	for (uint32_t i = 0; i < imageBuffer->imageCount; i++)
		kdo_freeGPUImage(&imageBuffer->imageArray[i], device);
	KDO_FREE(imageBuffer->imageArray);
	kdo_freeGPUMemory(&imageBuffer->memory, device);
	imageBuffer->fillSize	= 0;
	imageBuffer->imageCount	= 0;
}

void		kdo_freeCPUBuffer(Kdo_VkCPUBuffer *buffer)
{
	KDO_FREE(buffer->path);
	buffer->size		= 0;
	buffer->fillSize	= 0;
}

void		kdo_freeBSTNode(Kdo_VkBSTNode **node)
{
	if ((*node)->left != NULL)
		kdo_freeBSTNode(&((*node)->left));
	if ((*node)->right != NULL)
		kdo_freeBSTNode(&((*node)->right));
	KDO_FREE(*node);
}

void		kdo_freeBST(Kdo_VkBST *bst)
{
	kdo_freeBSTNode(&bst->root);
}

void		kdo_freeBuffer(Kdo_VkBuffer *buffer, VkDevice device)
{
	kdo_freeGPUBuffer(&buffer->GPU, device);
	kdo_freeCPUBuffer(&buffer->CPU);
}

void		kdo_freeSetBuffer(Kdo_VkSetBuffer *buffer, VkDevice device)
{
	kdo_freeGPUBuffer(&buffer->GPU, device);
	kdo_freeCPUBuffer(&buffer->CPU);
	kdo_freeBST(&buffer->BST);
}

void		kdo_freeSetImageBuffer(Kdo_VkSetImageBuffer *imageBuffer, VkDevice device)
{
	kdo_freeGPUImageBuffer(&imageBuffer->GPU, device);
	kdo_freeCPUBuffer(&imageBuffer->CPU);
	kdo_freeBST(&imageBuffer->BST);
}

Kdo_VkBSTNode   **kdo_BSTNodeFind(Kdo_VkBSTNode **root, void *buffer, VkDeviceSize dataSize, void *data)
{
	int	cmp;

	if (*root)
	{
		cmp = memcmp(buffer + (*root)->index * dataSize, data, dataSize);
		if (0 < cmp)
			return (kdo_BSTNodeFind(&(*root)->left, buffer, dataSize, data));
		else if (cmp < 0)
			return (kdo_BSTNodeFind(&(*root)->right, buffer, dataSize, data));
	}
	return (root);
}

Kdo_VkBSTNode   **kdo_BSTNodeStrFind(Kdo_VkBSTNode **root, char **strBuffer, char *str)
{
	int	cmp;

	if (*root)
	{
		cmp = strcmp(strBuffer[(*root)->index], str);
		if (0 < cmp)
			return (kdo_BSTNodeStrFind(&(*root)->left, strBuffer, str));
		else if (cmp < 0)
			return (kdo_BSTNodeStrFind(&(*root)->right, strBuffer, str));
	}
	return (root);
}

VkResult		kdo_BSTPush(Kdo_VkBST *bst, void *data, uint32_t *index)
{
	Kdo_VkBSTNode	**node;
	VkResult		returnCode;

	node = kdo_BSTNodeFind(&bst->root, bst->CPUBuffer->path, bst->elementSize, data);
	if (*node == NULL)
	{
		if (bst->CPUBuffer->size < bst->CPUBuffer->fillSize + bst->elementSize)
			KDO_VK_ASSERT(kdo_reallocCPUBuffer(bst->CPUBuffer, bst->CPUBuffer->fillSize + bst->elementSize));

		KDO_VK_ASSERT(kdo_newBSTNode(bst->CPUBuffer->fillSize / bst->elementSize, node));
		memcpy(bst->CPUBuffer->path + bst->CPUBuffer->fillSize, data, bst->elementSize);
		bst->CPUBuffer->fillSize += bst->elementSize;
	}
	*index	= (*node)->index;

	return (VK_SUCCESS);
}

VkResult		kdo_BSTStrPush(Kdo_VkBST *bst, char *str, uint32_t *index)
{
	Kdo_VkBSTNode	**node;
	VkResult		returnCode;

	node = kdo_BSTNodeStrFind(&bst->root, (char **)bst->CPUBuffer->path, str);
	if (*node == NULL)
	{
		if (bst->CPUBuffer->size < bst->CPUBuffer->fillSize + bst->elementSize)
			KDO_VK_ASSERT(kdo_reallocCPUBuffer(bst->CPUBuffer, bst->CPUBuffer->fillSize + bst->elementSize));

		KDO_VK_ASSERT(kdo_newBSTNode(bst->CPUBuffer->fillSize / bst->elementSize, node));
		((char **)bst->CPUBuffer->path)[(*node)->index] = str;
		bst->CPUBuffer->fillSize += bst->elementSize;
	}
	*index	= (*node)->index;

	return (VK_SUCCESS);
}

VkResult		kdo_reallocCPUBuffer(Kdo_VkCPUBuffer *buffer, VkDeviceSize newSize)
{
	if (!(buffer->path	= realloc(buffer->path, newSize)))
		return (VK_ERROR_OUT_OF_HOST_MEMORY);
	buffer->size		= newSize;
	buffer->fillSize	= kdo_minSize(buffer->fillSize, newSize);

	return (VK_SUCCESS);
}

VkResult		kdo_writeBufferData(Kdo_VkBuffer *buffer, VkDeviceSize offset, VkDeviceSize dataSize, void *data)
{
	return (VK_SUCCESS);
}

VkResult		kdo_pushBufferData(Kdo_VkBuffer *buffer, VkDeviceSize dataSize, void *data)
{
	return (VK_SUCCESS);
}

VkResult		kdo_pushSetBufferData(Kdo_VkSetBuffer *buffer, void *data, uint32_t *index)
{
	return (VK_SUCCESS);
}

VkResult		kdo_pushSetImageBufferPath(Kdo_VkSetImageBuffer *imageBuffer, char *path, uint32_t *index)
{
	return (VK_SUCCESS);
}

VkResult		kdo_beginUniqueCommand(VkDevice device, VkCommandPool commandPool, VkCommandBuffer *commandBuffer)
{
	VkCommandBufferAllocateInfo	allocInfo;
	VkCommandBufferBeginInfo	beginInfo;
	VkResult					returnCode;

	allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.pNext					= NULL;
	allocInfo.commandPool			= commandPool;
	allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount	= 1;
	KDO_VK_ASSERT(vkAllocateCommandBuffers(device, &allocInfo, commandBuffer));

	beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.pNext				= NULL;
	beginInfo.flags				= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo	= NULL;
	KDO_VK_ASSERT(vkBeginCommandBuffer(*commandBuffer, &beginInfo));

	return (VK_SUCCESS);
}

VkResult        kdo_findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t *memoryType)
{
	*memoryType = 0;
	while ( *memoryType < memoryProperties.memoryTypeCount)
	{
		if ((typeFilter & (1 << *memoryType)) && (memoryProperties.memoryTypes[*memoryType].propertyFlags & memoryFlags) == memoryFlags)
			return (VK_SUCCESS);

		*memoryType += 1;
	}
	return (VK_ERROR_UNKNOWN);
}

VkDeviceSize	kdo_minSize(VkDeviceSize val1, VkDeviceSize val2)
{
	if (val1 <= val2)
		return (val1);
	return (val2);
}

VkDeviceSize	kdo_maxSize(VkDeviceSize val1, VkDeviceSize val2)
{
	if (val1 <= val2)
		return (val2);
	return (val1);
}

VkResult		kdo_cmdImageBarrier(VkCommandBuffer commandBuffer, VkImage image, \
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

	return (VK_SUCCESS);
}

VkResult		kdo_endUniqueCommand(VkCommandBuffer *commandBuffer, VkDevice device, VkCommandPool commandPool, VkQueue queue)
{
	VkSubmitInfo	submitInfo;
	VkResult		returnCode;

	KDO_VK_ASSERT(vkEndCommandBuffer(*commandBuffer));

	submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext				= NULL;
	submitInfo.waitSemaphoreCount	= 0;
	submitInfo.pWaitSemaphores		= NULL;
	submitInfo.pWaitDstStageMask	= NULL;
	submitInfo.commandBufferCount	= 1;
	submitInfo.pCommandBuffers		= commandBuffer;
	submitInfo.signalSemaphoreCount	= 0;
	submitInfo.pSignalSemaphores	= NULL;
	KDO_VK_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, commandPool, 1, commandBuffer);

	return (VK_SUCCESS);
}

VkResult		kdo_writeHostGPUMemory(Kdo_VkGPUMemory *memory, VkDevice device, VkDeviceSize offset, VkDeviceSize dataSize, void *data)
{
	void            *mapMemory;
	VkResult		returnCode;

	if (dataSize == 0)
		return (VK_SUCCESS);

	if (memory->size < offset + dataSize)
		return (VK_ERROR_MEMORY_MAP_FAILED);

	KDO_VK_ASSERT(vkMapMemory(device, memory->path, offset, dataSize, 0, &mapMemory));
	memcpy(mapMemory, data, dataSize);
	vkUnmapMemory(device, memory->path);

	return (VK_SUCCESS);
}

VkResult		kdo_cpyGPUBuffer(Kdo_VkGPUBuffer *dstBuffer, Kdo_VkGPUBuffer *srcBuffer, VkDevice device, VkCommandPool commandPool, VkQueue queue, VkDeviceSize dstOffset, VkDeviceSize srcOffset, VkDeviceSize dataSize)
{
	VkCommandBuffer			commandBuffer;
	VkBufferCopy			copyInfo;
	VkResult				returnCode;

	if (dataSize == 0)
		return (VK_SUCCESS);

	if (srcBuffer->memory.size < srcOffset + dataSize || dstBuffer->memory.size < dstOffset + dataSize)
		return (VK_ERROR_MEMORY_MAP_FAILED);

	if (srcBuffer->fillSize < srcOffset + dataSize  || dstBuffer->fillSize < dstOffset)
		return (VK_ERROR_NOT_PERMITTED_EXT);

	KDO_VK_ASSERT(kdo_beginUniqueCommand(device, commandPool, &commandBuffer));
	copyInfo.srcOffset	= srcOffset;
	copyInfo.dstOffset	= dstOffset;
	copyInfo.size		= dataSize;
	vkCmdCopyBuffer(commandBuffer, srcBuffer->path, dstBuffer->path, 1, &copyInfo);
	KDO_VK_ASSERT(kdo_endUniqueCommand(&commandBuffer, device, commandPool, queue));

	dstBuffer->fillSize = kdo_maxSize(dstBuffer->fillSize, dstOffset + dataSize);

	return (VK_SUCCESS);
}

VkResult        kdo_reallocGPUBuffer(Kdo_VkGPUBuffer *buffer, VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, VkCommandPool commandPool, VkQueue queue, VkDeviceSize newSize)
{
	Kdo_VkGPUBuffer newBuffer;
	VkResult		returnCode;

	KDO_VK_ASSERT(kdo_newGPUBuffer(device, buffer->memory.memoryFlags, memoryProperties, buffer->memory.waitFlags, buffer->usage, newSize, &newBuffer));
	KDO_VK_ASSERT(kdo_cpyGPUBuffer(&newBuffer, buffer, device, commandPool, queue, 0, 0, newSize));

	return (VK_SUCCESS);
}

VkResult		kdo_writeGPUBuffer(Kdo_VkGPUBuffer *buffer, VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, VkCommandPool commandPool, VkQueue queue, VkDeviceSize offset, VkDeviceSize dataSize, void *data)
{
	Kdo_VkGPUBuffer	stagingBuffer;
	VkResult		returnCode;
	
	if (buffer->memory.size < offset + dataSize)
		return (VK_ERROR_MEMORY_MAP_FAILED);

	if (buffer->fillSize < offset)
		return (VK_ERROR_NOT_PERMITTED_EXT);

	if (buffer->memory.memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		KDO_VK_ASSERT(kdo_writeHostGPUMemory(&buffer->memory, device, offset, dataSize, data));
		buffer->fillSize = kdo_maxSize(buffer->fillSize, offset + dataSize);
	}
	else
	{
		KDO_VK_ASSERT(kdo_newGPUStagingBuffer(device, memoryProperties, dataSize, data, &stagingBuffer));
		KDO_VK_ASSERT(kdo_cpyGPUBuffer(buffer, &stagingBuffer, device, commandPool, queue, offset, 0, dataSize));
		kdo_freeGPUBuffer(&stagingBuffer, device);
	}

	return (VK_SUCCESS);
}

VkResult		kdo_pushGPUBuffer(Kdo_VkGPUBuffer *buffer, VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, VkCommandPool commandPool, VkQueue queue, VkDeviceSize dataSize, void *data)
{
	VkResult        returnCode;

	if (buffer->memory.size < buffer->fillSize + dataSize)
		kdo_reallocGPUBuffer(buffer, device, memoryProperties, commandPool, queue, buffer->fillSize + dataSize);

	KDO_VK_ASSERT(kdo_writeGPUBuffer(buffer, device, memoryProperties, commandPool, queue, buffer->fillSize, dataSize, data));

	return (VK_SUCCESS);
}

VkResult		kdo_reallocGPUImageBuffer(Kdo_VkGPUImageBuffer *imageBuffer, VkDevice device, VkCommandPool commandPool, VkQueue queue, void (*newImageInfo)(VkExtent3D, VkImageCreateInfo*), void (*newViewInfo)(VkImage, VkImageViewCreateInfo*), VkDeviceSize newSize)
{
	return (VK_SUCCESS);
}

VkResult		kdo_extendGPUImageBuffer(Kdo_VkGPUImageBuffer *imageBuffer, VkDevice device, VkCommandPool commandPool, VkQueue queue, void (*newImageInfo)(VkExtent3D, VkImageCreateInfo*), void (*newViewInfo)(VkImage, VkImageViewCreateInfo*), VkDeviceSize extendSize)
{
	return (VK_SUCCESS);
}

VkResult		kdo_pushGPUImage(Kdo_VkGPUImageBuffer *imageBuffer, Kdo_VkGPUImage *image, VkDevice device, VkCommandPool commandPool, VkQueue queue, void (*newImageInfo)(VkExtent3D, VkImageCreateInfo*), void (*newViewInfo)(VkImage, VkImageViewCreateInfo*))
{
	return (VK_SUCCESS);
}

VkResult		kdo_pushGPUImageFromGPUBuffer(Kdo_VkGPUImageBuffer *imageBuffer, Kdo_VkGPUBuffer *buffer, VkDevice device, VkCommandPool commandPool, VkQueue queue, void (*newImageInfo)(VkExtent3D, VkImageCreateInfo*), void (*newViewInfo)(VkImage, VkImageViewCreateInfo*), VkDeviceSize offset, VkExtent3D extent)
{
	return (VK_SUCCESS);
}

VkResult		kdo_updateAllBuffer(Kdo_VkBuffer *buffer, VkDevice device, VkCommandPool commandPool, VkQueue queue)
{
	return (VK_SUCCESS);
}

VkResult		kdo_updateBuffer(Kdo_VkBuffer *buffer, VkDevice device, VkCommandPool commandPool, VkQueue queue, VkDeviceSize offset, VkDeviceSize size)
{
	return (VK_SUCCESS);
}

VkResult		kdo_updateSetBuffer(Kdo_VkSetBuffer *buffer, VkDevice device, VkCommandPool commandPool, VkQueue queue)
{
	return (VK_SUCCESS);
}

VkResult		kdo_updateSetBufferImage(Kdo_VkSetImageBuffer imageBuffer, VkDevice device, VkCommandPool commandPool, VkQueue queue, void (*newImageInfo)(VkExtent3D, VkImageCreateInfo*), void (*newViewInfo)(VkImage, VkImageViewCreateInfo*))
{
	return (VK_SUCCESS);
}
