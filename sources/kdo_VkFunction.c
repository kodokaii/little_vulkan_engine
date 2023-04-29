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

void	kdo_createImage(Kdo_Vulkan *vk, Kdo_VkImageCreateInfo info, VkDeviceMemory *memory, VkImage *image)
{
	VkMemoryRequirements	memRequirements;
	VkMemoryAllocateInfo	allocInfo;

	if (vkCreateImage(vk->device.path, &info.path, NULL, image) != VK_SUCCESS)
		kdo_cleanup(vk, "Image creation failed", 16);

	vkGetImageMemoryRequirements(vk->device.path, *image, &memRequirements);

	allocInfo.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext				= NULL;
	allocInfo.allocationSize	= memRequirements.size;
	allocInfo.memoryTypeIndex	= kdo_findMemoryType(memRequirements.memoryTypeBits, info.memoryFlags, vk);
	if (vkAllocateMemory(vk->device.path, &allocInfo, NULL, memory) != VK_SUCCESS)
		kdo_cleanup(vk, "Memory allocation failed", 17);

	vkBindImageMemory(vk->device.path, *image, *memory, 0);
}
