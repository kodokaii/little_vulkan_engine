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

#include "kdo_VkOpenOBJ.h"

#define FAST_OBJ_IMPLEMENTATION
#include "objLoader/fast_obj.h"

static uint32_t	kdo_triangleCount(fastObjMesh *mesh)
{
	uint32_t	triangleCount;

	triangleCount = 0;
	for (uint32_t i = 0; i < mesh->face_count; i++)
		triangleCount += mesh->face_vertices[i] - 2;

	return (triangleCount);
}

static void	kdo_loadComponent(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, fastObjMesh *mesh)
{
	vec4			pos;
	vec4			normal;
	vec2			uv;
	Kdo_VkMaterial	material;
	char			*texture;
	uint32_t	i;


	pos[3]	= 0;
	for (i = 1; i < mesh->position_count; i++)
	{
		glm_vec3_copy(((vec3 *)mesh->positions)[i], pos);
		kdo_addPosObject(vk, objectInfo, i, pos);
	}

	normal[3]	= 0;
	for (i = 1; i < mesh->normal_count; i++)
	{
		glm_vec3_copy(((vec3 *)mesh->normals)[i], normal);
		kdo_addNormalObject(vk, objectInfo, i, normal);
	}
	for (i = 1; i < mesh->texcoord_count; i++)
	{
		glm_vec2_copy(((vec2 *)mesh->texcoords)[i], uv);
		kdo_addUvObject(vk, objectInfo, i, uv);
	}

	for (i = 1; i < mesh->texture_count; i++)
	{
		KDO_VK_ALLOC(texture, strdup(mesh->textures[i].path));
		kdo_addTextureObject(vk, objectInfo, i, texture);
	}

	for (i = 1; i < mesh->material_count; i++)
	{
		glm_vec3_make(mesh->materials[i].Ka, material.ambient);
		glm_vec3_make(mesh->materials[i].Kd, material.diffuse);
		glm_vec3_make(mesh->materials[i].Ks, material.specular);
		glm_vec3_make(mesh->materials[i].Ke, material.emissive);
		glm_vec3_make(mesh->materials[i].Kt, material.transmittance);
		glm_vec3_make(mesh->materials[i].Tf, material.transmissionFilter);
		material.ambientMap				= mesh->materials[i].map_Ka;
		material.diffuseMap				= mesh->materials[i].map_Kd;
		material.specularMap			= mesh->materials[i].map_Ks;
		material.emissiveMap			= mesh->materials[i].map_Ke;
		material.transmittanceMap		= mesh->materials[i].map_Kt;
		material.transmissionFilterMap	= 0;
		material.shininess				= mesh->materials[i].Ns;
		material.shininessMap			= mesh->materials[i].map_Ns;
		material.refractionIndex		= mesh->materials[i].Ni;
		material.refractionMap			= mesh->materials[i].map_Ni;
		material.disolve				= mesh->materials[i].d;
		material.disolveMap				= mesh->materials[i].map_d;
		material.illum					= mesh->materials[i].illum;
		material.bumpMap				= mesh->materials[i].map_bump;
		kdo_addMaterialObject(vk, objectInfo, i, &material);
	}
}

static void	kdo_loadVertex(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, fastObjMesh *mesh)
{
	Kdo_VkVertex	vertex[3];
	fastObjIndex	vertex1;
	fastObjIndex	vertex2;
	fastObjIndex	vertex3;
	uint32_t		currentFace;
	uint32_t		currentVertex;
	uint32_t		currentVertexInFace;

	vertex[0].tangentIndex		= 0;
	vertex[1].tangentIndex		= 0;
	vertex[2].tangentIndex		= 0;

	vertex[0].bitangentIndex	= 0;
	vertex[1].bitangentIndex	= 0;
	vertex[2].bitangentIndex	= 0;

	currentFace			= 0;
	currentVertex		= 0;
	while (currentFace < mesh->face_count)
	{
		vertex1	= mesh->indices[currentVertex];

		vertex[0].posIndex			= vertex1.p;
		vertex[0].normalIndex		= vertex1.n;
		vertex[0].uvIndex			= vertex1.t;
		vertex[0].mtlIndex			= mesh->face_materials[currentFace];
		vertex[1].mtlIndex			= mesh->face_materials[currentFace];
		vertex[2].mtlIndex			= mesh->face_materials[currentFace];

		currentVertexInFace	= 2;
		while (currentVertexInFace < mesh->face_vertices[currentFace])
		{
			vertex2	= mesh->indices[currentVertex + currentVertexInFace - 1];
			vertex3 = mesh->indices[currentVertex + currentVertexInFace];

			vertex[1].posIndex			= vertex2.p;
			vertex[1].normalIndex		= vertex2.n;
			vertex[1].uvIndex			= vertex2.t;

			vertex[2].posIndex			= vertex3.p;
			vertex[2].normalIndex		= vertex3.n;
			vertex[2].uvIndex			= vertex3.t;

			kdo_addTriangleObject(vk, objectInfo, vertex);

			currentVertexInFace++;
		}
		currentVertex	+= mesh->face_vertices[currentFace];
		currentFace++;
	}
}

VkResult    kdo_openObj(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, char *objPath)
{
	fastObjMesh		*mesh;
	
	if (!(mesh = fast_obj_read(objPath)))
		return (VK_ERROR_UNKNOWN);
	
	kdo_initObject(vk, objectInfo, kdo_triangleCount(mesh) * 3, mesh->position_count - 1, 0, 0, mesh->normal_count - 1, mesh->texcoord_count - 1, mesh->material_count - 1, mesh->texture_count - 1);

	kdo_loadComponent(vk, objectInfo, mesh);
	
	kdo_loadVertex(vk, objectInfo, mesh);

	fast_obj_destroy(mesh);

	return (VK_SUCCESS);
}
