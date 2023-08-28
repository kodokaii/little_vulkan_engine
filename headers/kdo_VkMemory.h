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

Kdo_VkBuffer    kdo_newBuffer(Kdo_Vulkan *vk, VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags, Kdo_VkWait waitFlags);
void			kdo_reallocBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferSrc, VkDeviceSize newSize);
void			kdo_setData(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer, void *data, VkDeviceSize dataSize, VkDeviceSize offset);
void			kdo_getData(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer, void *data, VkDeviceSize dataSize, VkDeviceSize offset);

#endif
