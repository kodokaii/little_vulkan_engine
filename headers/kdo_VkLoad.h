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

# include "kdo_Vulkan.h"
# include "kdo_VkOpenOBJ.h"
# include <libgen.h>

typedef struct Kdo_VkObjectInfoElement
{
	uint32_t		freeCount;
	uint32_t		*matchArray;
}	Kdo_VkObjectInfoElement;

typedef struct Kdo_VkObjectInfo
{
	Kdo_VkObjectInfoElement	pos;
	Kdo_VkObjectInfoElement	tangent;
	Kdo_VkObjectInfoElement	bitangent;
	Kdo_VkObjectInfoElement	normal;
	Kdo_VkObjectInfoElement	uv;
	Kdo_VkObjectInfoElement	material;
	Kdo_VkObjectInfoElement	texture;
	uint32_t				vertexCount;
	Kdo_VkVertex			*vertex;
	Kdo_VkObject			object;
}   Kdo_VkObjectInfo;

void				kdo_findNormal(vec3 pos1, vec3 pos2, vec3 pos3, vec3 normal);
void				kdo_findRawTangent(vec3 pos1, vec3 pos2, vec3 pos3, vec2 uv1, vec2 uv2, vec2 uv3, vec3 rawTangent);
void				kdo_findTangent(vec3 rawTangent, vec2 normal, vec3 tangent);
Kdo_VkMaterial		kdo_defaultMaterial(void);
void				kdo_loadObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo info);

#endif
