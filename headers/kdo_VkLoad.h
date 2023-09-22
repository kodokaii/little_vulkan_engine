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

#ifndef KDO_VKLOAD_H
# define KDO_VKLOAD_H

#include "kdo_Vulkan.h"
#include "kdo_VkOpenOBJ.h"

typedef struct Kdo_VkRawVertex
{
	vec3		pos;
	vec3		tangent;
	vec3		bitangent;
	vec3		normal;
	vec2		uv;
	uint32_t	mtl;
}	Kdo_VkRawVertex;

typedef struct Kdo_VkObjectInfo
{
	Kdo_VkRawVertex		*vertex;
    Kdo_ShMaterial      *material;
	char				**texturePath;
    uint32_t            vertexCount;
    uint32_t            materialCount;
	uint32_t			textureCount;
}   Kdo_VkObjectInfo;

Kdo_ShMaterial		kdo_defaultMaterial(void);
void				kdo_loadObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo info);

#endif
