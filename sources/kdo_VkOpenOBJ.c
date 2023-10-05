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

static Kdo_VkMaterial	kdo_getMaterial(fastObjMesh *mesh, uint32_t index)
{
	Kdo_VkMaterial	material;

	glm_vec3_make(mesh->materials[index].Ka, material.ambient);
	glm_vec3_make(mesh->materials[index].Kd, material.diffuse);
	glm_vec3_make(mesh->materials[index].Ks, material.specular);
	glm_vec3_make(mesh->materials[index].Ke, material.emissive);
	glm_vec3_make(mesh->materials[index].Kt, material.transmittance);
	glm_vec3_make(mesh->materials[index].Tf, material.transmissionFilter);
	material.ambientMap				= mesh->materials[index].map_Ka;
	material.diffuseMap				= mesh->materials[index].map_Kd;
	material.specularMap			= mesh->materials[index].map_Ks;
	material.emissiveMap			= mesh->materials[index].map_Ke;
	material.transmittanceMap		= mesh->materials[index].map_Kt;
	material.transmissionFilterMap	= 0;
	material.shininess				= mesh->materials[index].Ns;
	material.refractionIndex		= mesh->materials[index].Ni;
	material.disolve				= mesh->materials[index].d;
	material.illum					= mesh->materials[index].illum;

	return (material);
}

static char	*kdo_getTexture(Kdo_Vulkan *vk, fastObjMesh *mesh, uint32_t index)
{
	char	*texturePath;

	KDO_VK_ALLOC(texturePath, strdup(mesh->textures[index].path));

	return (texturePath);
}

static void	kdo_loadVertex(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, fastObjMesh *mesh)
{
	Kdo_VkVertex	vertex[3] = {};
	fastObjIndex	vertex1;
	fastObjIndex	vertex2;
	fastObjIndex	vertex3;
	uint32_t		currentFace;
	uint32_t		currentVertex;
	uint32_t		currentVertexInFace;


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
	uint32_t		i;
	
	mesh = fast_obj_read(objPath);
	
	kdo_initObject(vk, objectInfo, kdo_triangleCount(mesh) * 3, mesh->position_count, 0, 0, mesh->normal_count, mesh->texcoord_count, mesh->material_count, mesh->texture_count);

	for (i = 1; i < mesh->position_count; i++)
		kdo_addPosObject(vk, objectInfo, i, ((vec3 *)mesh->positions)[i]);
	for (i = 1; i < mesh->normal_count; i++)
		kdo_addNormalObject(vk, objectInfo, i, ((vec3 *)mesh->normals)[i]);
	for (i = 1; i < mesh->texcoord_count; i++)
		kdo_addUvObject(vk, objectInfo, i, ((vec2 *)mesh->texcoords)[i]);
	for (i = 1; i < mesh->material_count; i++)
		kdo_addMaterialObject(vk, objectInfo, i, kdo_getMaterial(mesh, i));
	for (i = 1; i < mesh->texture_count; i++)
		kdo_addTextureObject(vk, objectInfo, i, kdo_getTexture(vk, mesh, i));
	
	kdo_loadVertex(vk, objectInfo, mesh);

	fast_obj_destroy(mesh);

	return (VK_SUCCESS);
}
