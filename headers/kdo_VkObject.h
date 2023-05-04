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

#ifndef KDO_VKOBJECT_H
# define KDO_VKOBJECT_H

# include "kdo_Vulkan.h"

void			kdo_allocBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer, VkMemoryPropertyFlags memoryFlags);
Kdo_VkBuffer    kdo_mergeBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferSrc1, Kdo_VkBuffer *bufferSrc2);
void			kdo_addData(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferDst, uint32_t infoCount, Kdo_VkAddDataInfo *info);
void			kdo_loadMesh(Kdo_Vulkan *vk, Kdo_VkBuffer *vertexBuffer, Kdo_VkBuffer *indexBuffer, uint32_t infoCount, Kdo_VkLoadMeshInfo *info);
void			kdo_freeBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer);

#endif
