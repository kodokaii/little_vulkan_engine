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
#include <libgen.h>

typedef struct Kdo_VkObjectInfo
{
    uint32_t        freeVertexCount;
	uint32_t		freePosCount;
	uint32_t		freeTangentCount;
	uint32_t		freeBitangentCount;
	uint32_t		freeNormalCount;
	uint32_t		freeUvCount;
	uint32_t		freeMaterialCount;
	uint32_t		freeTextureCount;
	uint32_t		*posMatchArray;
	uint32_t		*tangentMatchArray;
	uint32_t		*bitangentMatchArray;
	uint32_t		*normalMatchArray;
	uint32_t		*uvMatchArray;
	uint32_t		*materialMatchArray;
	uint32_t		*textureMatchArray;
	Kdo_VkObject	object;
}   Kdo_VkObjectInfo;

void				kdo_findNormal(vec3 pos1, vec3 pos2, vec3 pos3, vec3 normal);
void				kdo_findRawTangent(vec3 pos1, vec3 pos2, vec3 pos3, vec2 uv1, vec2 uv2, vec2 uv3, vec3 rawTangent);
void				kdo_findTangent(vec3 rawTangent, vec2 normal, vec3 tangent);
Kdo_VkMaterial		kdo_defaultMaterial(void);
void				kdo_loadObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo info);

#endif
