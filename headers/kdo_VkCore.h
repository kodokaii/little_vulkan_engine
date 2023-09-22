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

typedef struct Kdo_VkObjectInfo
{
    Kdo_VkVertex        *vertex;
	vec3				*vector3;
	vec2				*vector2;
    Kdo_ShMaterial      *material;
	char				**texturePath;
    uint32_t            vertexCount;
	uint32_t			vector3Count;
	uint32_t			vector2Count;
    uint32_t            materialCount;
	uint32_t			textureCount;
}   Kdo_VkObjectInfo;

void				kdo_initCore(Kdo_Vulkan *vk);
uint32_t			kdo_loadMesh(Kdo_Vulkan *vk, Kdo_Vertex *vertexIn, uint32_t vertexInCount);
uint32_t			kdo_loadTexture(Kdo_Vulkan *vk, char *texturePath);
void				kdo_loadObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo info);
Kdo_VkObjectInfo	kdo_openObj(Kdo_Vulkan *vk, char *objPath);

#endif
