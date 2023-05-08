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
void			kdo_allocImage(Kdo_Vulkan *vk, Kdo_VkImage *image, VkMemoryPropertyFlags memoryFlags, Kdo_VkImageInfoFunc func);
Kdo_VkBuffer    kdo_mergeBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferSrc1, Kdo_VkBuffer *bufferSrc2);
Kdo_VkImage		kdo_mergeImage(Kdo_Vulkan *vk, Kdo_VkImage *imageSrc1, Kdo_VkImage *imageSrc2, Kdo_VkMergeImageInfo info);
Kdo_VkImage		kdo_mergeBufferToImage(Kdo_Vulkan *vk, Kdo_VkBuffer *bufferSrc, Kdo_VkImage *imageSrc, Kdo_VkMergeBufferToImageInfo info);
Kdo_VkBuffer    kdo_loadData(Kdo_Vulkan *vk, uint32_t infoCount, Kdo_VkLoadDataInfo *info);
void			kdo_loadMesh(Kdo_Vulkan *vk, Kdo_VkBuffer *vertexBuffer, Kdo_VkBuffer *indexBuffer, uint32_t infoCount, Kdo_VkLoadMeshInfo *info);
void			kdo_loadTextures(Kdo_Vulkan *vk, Kdo_VkImage *textures, uint32_t texturesCount, char **texturesPath);
void		    kdo_loadObject(Kdo_Vulkan *vk, Kdo_VkObject *object, uint32_t infoCount, Kdo_VkLoadObjectInfo *info);
void			kdo_updateDescripteur(Kdo_Vulkan *vk, Kdo_VkObject *object);
Kdo_VkObjectDiv *kdo_getObject(Kdo_VkObject *object, uint32_t index);
void			kdo_changeObjectCount(Kdo_Vulkan *vk, Kdo_VkObject *object, uint32_t index, uint32_t count);
void			kdo_updateModel(Kdo_VkTransform *model, uint32_t count);

#endif
