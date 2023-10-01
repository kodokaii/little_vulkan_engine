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

#ifndef KDO_VKMEMORY_H
# define KDO_VKMEMORY_H

# include <vulkan/vulkan.h>

typedef enum Kdo_VkWait
{
	WAIT_NONE	= 0x00000000,
	WAIT_DEVICE	= 0x00000001
}	Kdo_VkWait;

typedef struct Kdo_VkGPUMemory
{
    VkDeviceMemory			path;
	VkMemoryPropertyFlags	memoryFlags;
	Kdo_VkWait				waitFlags;
    VkDeviceSize			size;
}	Kdo_VkGPUMemory;

typedef struct Kdo_VkGPUBuffer
{
    VkBuffer			path;
	Kdo_VkGPUMemory		memory;
    VkBufferUsageFlags	usage;
	VkDeviceSize		fillSize;
}	Kdo_VkGPUBuffer;

typedef struct Kdo_VkGPUImage
{
    VkImage			path;
    VkImageView		view;
    VkExtent3D		extent;
    VkDeviceSize	offset;
}	Kdo_VkGPUImage;

typedef struct Kdo_VkGPUImageBuffer
{
    Kdo_VkGPUImage	*imageArray;
    Kdo_VkGPUMemory	memory;
    VkImageLayout	layout;
	VkDeviceSize	fillSize;
	uint32_t		memoryFilter;
	uint32_t		imageCount;
}	Kdo_VkGPUImageBuffer;

typedef struct Kdo_VkCPUBuffer
{
    void			*path;
	VkDeviceSize	size;
	VkDeviceSize	fillSize;
}	Kdo_VkCPUBuffer;

typedef struct Kdo_VkBSTNode
{
    struct Kdo_VkBSTNode	*left;
    struct Kdo_VkBSTNode	*right;
    uint32_t				index;
}   Kdo_VkBSTNode;

typedef struct Kdo_VkBST
{
	Kdo_VkBSTNode	*root;
	Kdo_VkCPUBuffer	*CPUBuffer;
	VkDeviceSize	elementSize;
}	Kdo_VkBST;

typedef	struct Kdo_VkBuffer
{
    Kdo_VkGPUBuffer	GPU;
    Kdo_VkCPUBuffer	CPU;
}	Kdo_VkBuffer;

typedef struct Kdo_VkSetBuffer
{
    Kdo_VkGPUBuffer	GPU;
    Kdo_VkCPUBuffer	CPU;
	Kdo_VkBST		BST;
}	Kdo_VkSetBuffer;

typedef struct	Kdo_VkSetImageBuffer
{
    Kdo_VkGPUImageBuffer	GPU;
    Kdo_VkCPUBuffer			CPU;
    Kdo_VkBST				BST;
}	Kdo_VkSetImageBuffer;

# define KDO_VK_ASSERT(function) if ((returnCode = function) != VK_SUCCESS) return (returnCode)
# define KDO_VK_FREE(function, device, handle) if (handle != VK_NULL_HANDLE) {function(device, handle, NULL); handle = VK_NULL_HANDLE;}
# define KDO_FREE(pointer) if (pointer != NULL) {free(pointer); pointer = NULL;}

VkResult		kdo_newGPUMemory(VkDevice device, uint32_t memoryFilter, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, Kdo_VkWait waitFlags, VkDeviceSize size, Kdo_VkGPUMemory *memory);
VkResult		kdo_newGPUBuffer(VkDevice device, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, Kdo_VkWait waitFlags, VkBufferUsageFlags usage, VkDeviceSize size, Kdo_VkGPUBuffer *buffer);
VkResult		kdo_newGPUStagingBuffer(VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, VkDeviceSize dataSize, void *data, Kdo_VkGPUBuffer *stagingBuffer);
VkResult        kdo_newGPUImage(VkDevice device, VkExtent3D extent, void (*newImageInfo)(VkExtent3D, VkImageCreateInfo*), void (*newViewInfo)(VkImage, VkImageViewCreateInfo*), VkDeviceSize offset, Kdo_VkGPUMemory *memory, Kdo_VkGPUImage *image);
VkResult		kdo_newGPUImageBuffer(VkDevice device, uint32_t memoryFilter, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, VkImageLayout layout, Kdo_VkWait waitFlags, VkDeviceSize size, Kdo_VkGPUImageBuffer *imageBuffer);
VkResult		kdo_newCPUBuffer(VkDeviceSize size, Kdo_VkCPUBuffer *buffer);
VkResult		kdo_newBSTNode(uint32_t index, Kdo_VkBSTNode **node);
VkResult		kdo_newBST(VkDeviceSize elementSize, void **bufferPointer, Kdo_VkBST *bst);
VkResult		kdo_newBuffer(VkDevice device, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, Kdo_VkWait waitFlags, VkBufferUsageFlags usage, VkDeviceSize size, Kdo_VkBuffer *buffer);
VkResult		kdo_newSetBuffer(VkDevice device, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, Kdo_VkWait waitFlags, VkBufferUsageFlags usage, VkDeviceSize elementSize, VkDeviceSize size, Kdo_VkSetBuffer *buffer);
VkResult		kdo_newSetImageBuffer(VkDevice device, uint32_t memoryFilter, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, VkImageLayout layout, Kdo_VkWait waitFlags, VkDeviceSize size, Kdo_VkSetImageBuffer *imageBuffer);

void			kdo_freeGPUMemory(Kdo_VkGPUMemory *memory, VkDevice device);
void			kdo_freeGPUBuffer(Kdo_VkGPUBuffer *buffer, VkDevice device);
void			kdo_freeGPUImage(Kdo_VkGPUImage *image, VkDevice device);
void			kdo_freeGPUImageBuffer(Kdo_VkGPUImageBuffer *imageBuffer, VkDevice device);
void			kdo_freeCPUBuffer(Kdo_VkCPUBuffer *buffer);
void			kdo_freeBSTNode(Kdo_VkBSTNode **node);
void			kdo_freeBST(Kdo_VkBST *bst);
void			kdo_freeBuffer(Kdo_VkBuffer *buffer, VkDevice device);
void			kdo_freeSetBuffer(Kdo_VkSetBuffer *buffer, VkDevice device);
void			kdo_freeSetImageBuffer(Kdo_VkSetImageBuffer *imageBuffer, VkDevice device);

Kdo_VkBSTNode	**kdo_BSTNodeFind(Kdo_VkBSTNode **root, void *buffer, VkDeviceSize dataSize, void *data);
Kdo_VkBSTNode   **kdo_BSTNodeStrFind(Kdo_VkBSTNode **root, char **strBuffer, char *str);
VkResult        kdo_BSTPush(Kdo_VkBST *bst, void *data, uint32_t *index);
VkResult        kdo_BSTStrPush(Kdo_VkBST *bst, char *str, uint32_t *index);

VkResult		kdo_reallocCPUBuffer(Kdo_VkCPUBuffer *buffer, VkDeviceSize newSize);
VkResult		kdo_writeBufferData(Kdo_VkBuffer *buffer, VkDeviceSize offset, VkDeviceSize dataSize, void *data);	
VkResult		kdo_pushBufferData(Kdo_VkBuffer *buffer, VkDeviceSize dataSize, void *data);
VkResult		kdo_pushSetBufferData(Kdo_VkSetBuffer *buffer, void *data, uint32_t *index);
VkResult		kdo_pushSetImageBufferPath(Kdo_VkSetImageBuffer *imageBuffer, char *path, uint32_t *index);

VkResult		kdo_beginUniqueCommand(VkDevice device, VkCommandPool commandPool, VkCommandBuffer *commandBuffer);
VkResult		kdo_findMemoryType(uint32_t memoryFilter, VkMemoryPropertyFlags memoryFlags, VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t *memoryType);
VkDeviceSize    kdo_minSize(VkDeviceSize val1, VkDeviceSize val2);
VkDeviceSize    kdo_maxSize(VkDeviceSize val1, VkDeviceSize val2);
VkResult		kdo_cmdImageBarrier(VkCommandBuffer commandBuffer, VkImage image, \
								    VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, \
									VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, \
									VkImageLayout oldLayout, VkImageLayout newLayout);
VkResult        kdo_endUniqueCommand(VkCommandBuffer *commandBuffer, VkDevice device, VkCommandPool commandPool, VkQueue queue);

VkResult        kdo_writeHostGPUMemory(Kdo_VkGPUMemory *memory, VkDevice device, VkDeviceSize offset, VkDeviceSize dataSize, void *data);
VkResult        kdo_cpyGPUBuffer(Kdo_VkGPUBuffer *dstBuffer, Kdo_VkGPUBuffer *srcBuffer, VkDevice device, VkCommandPool commandPool, VkQueue queue, VkDeviceSize dstOffset, VkDeviceSize srcOffset, VkDeviceSize dataSize);
VkResult		kdo_reallocGPUBuffer(Kdo_VkGPUBuffer *buffer, VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, VkCommandPool commandPool, VkQueue queue, VkDeviceSize newSize);
VkResult        kdo_writeGPUBuffer(Kdo_VkGPUBuffer *buffer, VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties,VkCommandPool commandPool, VkQueue queue, VkDeviceSize offset, VkDeviceSize dataSize, void *data);
VkResult        kdo_pushGPUBuffer(Kdo_VkGPUBuffer *buffer, VkDevice device, VkPhysicalDeviceMemoryProperties memoryProperties, VkCommandPool commandPool, VkQueue queue, VkDeviceSize dataSize, void *data);
VkResult		kdo_reallocGPUImageBuffer(Kdo_VkGPUImageBuffer *imageBuffer, VkDevice device, VkCommandPool commandPool, VkQueue queue, void (*newImageInfo)(VkExtent3D, VkImageCreateInfo*), void (*newViewInfo)(VkImage, VkImageViewCreateInfo*), VkDeviceSize newSize); 
VkResult		kdo_extendGPUImageBuffer(Kdo_VkGPUImageBuffer *imageBuffer, VkDevice device, VkCommandPool commandPool, VkQueue queue, void (*newImageInfo)(VkExtent3D, VkImageCreateInfo*), void (*newViewInfo)(VkImage, VkImageViewCreateInfo*), VkDeviceSize extendSize); 
VkResult		kdo_pushGPUImage(Kdo_VkGPUImageBuffer *imageBuffer, Kdo_VkGPUImage *image, VkDevice device, VkCommandPool commandPool, VkQueue queue, void (*newImageInfo)(VkExtent3D, VkImageCreateInfo*), void (*newViewInfo)(VkImage, VkImageViewCreateInfo*));
VkResult		kdo_pushGPUImageFromGPUBuffer(Kdo_VkGPUImageBuffer *imageBuffer, Kdo_VkGPUBuffer *buffer, VkDevice device, VkCommandPool commandPool, VkQueue queue, void (*newImageInfo)(VkExtent3D, VkImageCreateInfo*), void (*newViewInfo)(VkImage, VkImageViewCreateInfo*), VkDeviceSize offset, VkExtent3D extent);

VkResult		kdo_updateAllBuffer(Kdo_VkBuffer *buffer, VkDevice device, VkCommandPool commandPool, VkQueue queue);
VkResult		kdo_updateBuffer(Kdo_VkBuffer *buffer, VkDevice device, VkCommandPool commandPool, VkQueue queue, VkDeviceSize offset, VkDeviceSize size);
VkResult		kdo_updateSetBuffer(Kdo_VkSetBuffer *buffer, VkDevice device, VkCommandPool commandPool, VkQueue queue);
VkResult		kdo_updateSetBufferImage(Kdo_VkSetImageBuffer imageBuffer, VkDevice device, VkCommandPool commandPool, VkQueue queue, void (*newImageInfo)(VkExtent3D, VkImageCreateInfo*), void (*newViewInfo)(VkImage, VkImageViewCreateInfo*));

#endif
