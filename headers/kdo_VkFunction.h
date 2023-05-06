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

#ifndef KDO_VKFUNCTION_H
# define KDO_VKFUNCTION_H

# include "kdo_Vulkan.h"

int			kdo_checkExtensions(const char **find, uint32_t findCount, const char *list, uint32_t listCount, size_t offsetList);
VkFormat    kdo_findFormat(Kdo_Vulkan *vk, VkFormat *formats, uint32_t formatsCount, VkImageTiling tiling, VkFormatFeatureFlags features);
uint32_t    kdo_findMemoryType(Kdo_Vulkan *vk, uint32_t typeFilter, VkMemoryPropertyFlags memoryFlags);
void		kdo_beginUniqueCommand(Kdo_Vulkan *vk, VkCommandBuffer *commandBuffer);
void		kdo_endUniqueCommand(Kdo_Vulkan *vk, VkCommandBuffer *commandBuffer);
void		kdo_imageTextureInfo(VkExtent3D extent, VkImageCreateInfo *imageInfo);
uint32_t    findTextureMemoryFiltrer(Kdo_Vulkan *vk);
void		*kdo_mallocMerge(size_t sizeSrc1, void *src1, size_t sizeSrc2, void *src2);

#endif
