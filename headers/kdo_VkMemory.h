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

typedef struct Kdo_VkImageFuncInfo
{
	VkDeviceSize	offset;
	VkExtent3D		extent;
	void			(*imageInfo)(VkExtent3D, VkImageCreateInfo*);
	void			(*viewInfo)(VkImage, VkImageViewCreateInfo*);
}	Kdo_VkImageFuncInfo;

Kdo_VkBuffer		kdo_newBuffer(Kdo_Vulkan *vk, VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags, Kdo_VkWait waitFlags);
void				kdo_reallocBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferSrc, VkDeviceSize newSize);
void				kdo_setData(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer, void *data, VkDeviceSize dataSize, VkDeviceSize offset);
void				kdo_getData(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer, void *data, VkDeviceSize dataSize, VkDeviceSize offset);

Kdo_VkImageBuffer	kdo_newImageBuffer(Kdo_Vulkan *vk, VkDeviceSize bufferSize, VkImageLayout layout, VkMemoryPropertyFlags memoryFlags, uint32_t memoryFilter, Kdo_VkWait waitFlags);
void				kdo_reallocImageBuffer(Kdo_Vulkan *vk, Kdo_VkImageBuffer *imageBufferSrc, VkDeviceSize newSize, Kdo_VkImageFuncInfo funcInfo);
void				kdo_appendImageFromImage(Kdo_Vulkan *vk, Kdo_VkImageBuffer *imageBufferDst, Kdo_VkImageBuffer *imageBufferSrc, uint32_t imageIndex, Kdo_VkImageFuncInfo funcInfo);
void				kdo_appendImageFromBuffer(Kdo_Vulkan *vk, Kdo_VkImageBuffer *imageBufferDst, Kdo_VkBuffer *bufferSrc, VkDeviceSize offset, VkExtent3D extent, Kdo_VkImageFuncInfo funcInfo);
void				kdo_appendImage(Kdo_Vulkan *vk, Kdo_VkImageBuffer *imageBufferDst, char *imagePath, Kdo_VkImageFuncInfo funcInfo);

#endif
