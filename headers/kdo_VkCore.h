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

#ifndef KDO_VKRENDER_H
# define KDO_VKRENDER_H

# include "kdo_Vulkan.h"
# include "kdo_VkMemory.h"

typedef struct Kdo_VkLoadMeshInfo
{
	uint32_t	count;
	Kdo_Vertex	*vertex;
}	Kdo_VkLoadMeshInfo;

typedef struct Kdo_VkObjectInfo
{
    char                *name;
    uint32_t            vertexCount;
    Kdo_Vertex          *vertex;
    uint32_t            materialCount;
    Kdo_ShMaterial      *material;
}   Kdo_VkObjectInfo;

void				kdo_initCore(Kdo_Vulkan *vk);
void				kdo_loadMesh(Kdo_Vulkan *vk, uint32_t infoCount, Kdo_VkLoadMeshInfo *info);
void				kdo_loadTextures(Kdo_Vulkan *vk, uint32_t texturesCount, char **texturesPath);
void				kdo_loadObject(Kdo_Vulkan *vk, uint32_t objectCount, Kdo_VkObjectInfo *info);
Kdo_VkObjectInfo	kdo_openObj(Kdo_Vulkan *vk, char *objPath);

#endif
