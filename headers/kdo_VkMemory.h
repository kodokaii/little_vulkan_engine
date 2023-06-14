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

#include "kdo_Vulkan.h"

typedef struct Kdo_VkImageInfoFunc
{
	void			(*imageInfo)(VkExtent3D, VkImageCreateInfo*);
	void			(*viewInfo)(VkImage,VkImageViewCreateInfo*);
}	Kdo_VkImageInfoFunc;

typedef struct Kdo_VkCatImageInfo
{
	Kdo_VkImageInfoFunc func;
	VkImageLayout       layout;
}	Kdo_VkCatImageInfo;

typedef struct Kdo_VkCatBufferToImageInfo
{
	Kdo_VkImageInfoFunc	func;
	VkImageLayout		layout;
	uint32_t			imagesCount;
	VkExtent3D			*extents;
}	Kdo_VkCatBufferToImageInfo;

typedef	struct Kdo_VkLoadDataInfo
{
	VkDeviceSize	elementSize;
	uint32_t		count;
	void			*data;
}	Kdo_VkLoadDataInfo;

void			kdo_allocBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer, VkMemoryPropertyFlags memoryFlags);
void			kdo_allocImage(Kdo_Vulkan *vk, Kdo_VkImage *image, VkMemoryPropertyFlags memoryFlags, Kdo_VkImageInfoFunc func);
Kdo_VkBuffer    kdo_copyBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferSrc, VkMemoryPropertyFlags memoryFlags);
Kdo_VkBuffer    kdo_catBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferSrc1, Kdo_VkBuffer *bufferSrc2, VkMemoryPropertyFlags memoryFlags);
Kdo_VkImage		kdo_catImage(Kdo_Vulkan *vk, Kdo_VkImage *imageSrc1, Kdo_VkImage *imageSrc2, Kdo_VkCatImageInfo info);
Kdo_VkImage		kdo_catBufferToImage(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferSrc, Kdo_VkImage *imageSrc, Kdo_VkCatBufferToImageInfo info);
Kdo_VkBuffer    kdo_loadData(Kdo_Vulkan *vk, uint32_t infoCount, Kdo_VkLoadDataInfo *info);

#endif
