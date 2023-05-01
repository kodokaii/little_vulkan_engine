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
VkBuffer    kdo_createBuffer(Kdo_Vulkan *vk, VkDeviceSize size, VkBufferUsageFlags usage);
VkImage		kdo_createImageTexture(Kdo_Vulkan *vk, int width, int height);

#endif
