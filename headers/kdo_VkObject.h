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
# include "kdo_VkMemory.h"

void			kdo_loadMesh(Kdo_Vulkan *vk, Kdo_VkBuffer *vertexBuffer, Kdo_VkBuffer *indexBuffer, uint32_t infoCount, Kdo_VkLoadMeshInfo *info);
void			kdo_loadTextures(Kdo_Vulkan *vk, Kdo_VkImage *textures, uint32_t texturesCount, char **texturesPath);
void		    kdo_loadObject(Kdo_Vulkan *vk, Kdo_VkObject *object, uint32_t infoCount, Kdo_VkLoadObjectInfo *info);
Kdo_Vertex		*kdo_openObj(char *objPath, uint32_t *vertexCount);
Kdo_VkObjectDiv *kdo_getObject(Kdo_VkObject *object, uint32_t index);
void			kdo_changeObjectCount(Kdo_Vulkan *vk, Kdo_VkObjectDiv *objectDiv, uint32_t newObjectCount);
void			kdo_updateDescripteur(Kdo_Vulkan *vk, Kdo_VkObject *object);
void			kdo_updateTransform(Kdo_VkTransform *transform, uint32_t count);

#endif
