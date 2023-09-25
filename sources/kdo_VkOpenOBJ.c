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

static void	kdo_cpyMaterial(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, fastObjMesh *mesh)
{
	if (!(objectInfo->material		= malloc(glm_max(mesh->material_count, 1) * sizeof(Kdo_ShMaterial))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(objectInfo->texturePath	= calloc(glm_max(mesh->material_count * 3, 1), sizeof(char *))))
		kdo_cleanup(vk, ERRLOC, 12);

	if (!mesh->material_count)
		objectInfo->material[0] = kdo_defaultMaterial();
	else
	{
		for (uint32_t i = 0; i < mesh->material_count; i++)
		{
			glm_vec3_make(mesh->materials[i].Ka, objectInfo->material[i].ambient);
			glm_vec3_make(mesh->materials[i].Kd, objectInfo->material[i].diffuse);
			glm_vec3_make(mesh->materials[i].Ks, objectInfo->material[i].specular);
			glm_vec3_make(mesh->materials[i].Ke, objectInfo->material[i].emissive);
			glm_vec3_make(mesh->materials[i].Kt, objectInfo->material[i].transmittance);
			glm_vec3_make(mesh->materials[i].Tf, objectInfo->material[i].transmissionFilter);
			objectInfo->material[i].shininess			= mesh->materials[i].Ns;
			objectInfo->material[i].refractionIndex		= mesh->materials[i].Ni;
			objectInfo->material[i].disolve				= mesh->materials[i].d;
			objectInfo->material[i].illum				= mesh->materials[i].illum;

			objectInfo->material[i].diffuseMap		= i * 3;
			objectInfo->material[i].specularMap		= i * 3 + 1;
			objectInfo->material[i].bumpMap			= i * 3 + 2;

			if (mesh->materials[i].map_Kd.path)
				if (!(objectInfo->texturePath[i * 3]		= strdup(mesh->materials[i].map_Kd.path)))
					kdo_cleanup(vk, ERRLOC, 12);

			if (mesh->materials[i].map_Ks.path)
				if (!(objectInfo->texturePath[i * 3 + 1]	= strdup(mesh->materials[i].map_Ks.path)))
					kdo_cleanup(vk, ERRLOC, 12);

			if (mesh->materials[i].map_bump.path)
				if (!(objectInfo->texturePath[i * 3 + 2]	= strdup(mesh->materials[i].map_bump.path)))
					kdo_cleanup(vk, ERRLOC, 12);
		}
	}
}

static uint32_t	kdo_triangleCount(fastObjMesh *mesh)
{
	uint32_t	triangleCount;

	triangleCount = 0;
	for (uint32_t i = 0; i < mesh->face_count; i++)
		triangleCount += glm_max(mesh->face_vertices[i] - 2, 0);

	return (triangleCount);
}

static void	kdo_fanTriangulation(Kdo_VkObjectInfo *objectInfo, fastObjMesh *mesh, uint32_t *currentVertex, uint32_t currentFace)
{
	fastObjIndex	mainVertex;
	fastObjIndex	vertex2;
	fastObjIndex	vertex3;
	vec3			rawTangent;
	uint32_t		vertexCount;

	mainVertex	= mesh->indices[*currentVertex];
	for (vertexCount = 2; vertexCount < mesh->face_vertices[currentFace]; vertexCount++)
	{
		vertex2	= mesh->indices[*currentVertex + vertexCount - 1];
		vertex3 = mesh->indices[*currentVertex + vertexCount];

		glm_vec3_make(&mesh->positions[mainVertex.p], objectInfo->vec3Array[*currentVertex * 4]);
		glm_vec3_make(&mesh->positions[vertex2.p], objectInfo->vec3Array[*currentVertex * 4 + 1]);
		glm_vec3_make(&mesh->positions[vertex3.p], objectInfo->vec3Array[*currentVertex * 4 + 2]);
		objectInfo->vertex[*currentVertex + 0].posIndex	= *currentVertex * 4;
		objectInfo->vertex[*currentVertex + 1].posIndex	= *currentVertex * 4 + 1;
		objectInfo->vertex[*currentVertex + 2].posIndex	= *currentVertex * 4 + 2;

		glm_vec2_make(&mesh->texcoords[mainVertex.t], objectInfo->vec2Array[*currentVertex]);
		glm_vec2_make(&mesh->texcoords[vertex2.t], objectInfo->vec2Array[*currentVertex + 1]);
		glm_vec2_make(&mesh->texcoords[vertex3.t], objectInfo->vec2Array[*currentVertex + 2]);

		kdo_findRawTangent(objectInfo->vec3Array[*currentVertex * 4], objectInfo->vec3Array[*currentVertex * 4 + 1], objectInfo->vec3Array[*currentVertex * 4 + 2], \
							objectInfo->vec2Array[*currentVertex], objectInfo->vec2Array[*currentVertex + 1], objectInfo->vec2Array[*currentVertex + 2], rawTangent);
		if (1 < mesh->normal_count)
		{
			glm_vec3_make(&mesh->normals[mainVertex.n], vertexDst[0].normal);
			glm_vec3_make(&mesh->normals[vertex2.n], vertexDst[1].normal);
			glm_vec3_make(&mesh->normals[vertex3.n], vertexDst[2].normal);

			kdo_findTangent(rawTangent, vertexDst[0].normal, vertexDst[0].tangent);
			kdo_findTangent(rawTangent, vertexDst[1].normal, vertexDst[1].tangent);
			kdo_findTangent(rawTangent, vertexDst[2].normal, vertexDst[2].tangent);
		
			glm_vec3_crossn(vertexDst[0].normal, vertexDst[0].tangent, vertexDst[0].bitangent);
			glm_vec3_crossn(vertexDst[1].normal, vertexDst[1].tangent, vertexDst[1].bitangent);
			glm_vec3_crossn(vertexDst[2].normal, vertexDst[2].tangent, vertexDst[2].bitangent);
		}
		else
		{
			kdo_findNormal(vertexDst[0].pos, vertexDst[1].pos, vertexDst[2].pos, vertexDst[0].normal);
			glm_vec3_dup(vertexDst[0].normal, vertexDst[1].normal);
			glm_vec3_dup(vertexDst[0].normal, vertexDst[2].normal);

			glm_vec3_dup(rawTangent, vertexDst[0].tangent);
			glm_vec3_dup(rawTangent, vertexDst[1].tangent);
			glm_vec3_dup(rawTangent, vertexDst[2].tangent);

			glm_vec3_crossn(vertexDst[0].normal, vertexDst[0].tangent, vertexDst[0].bitangent);
			glm_vec3_dup(vertexDst[0].bitangent, vertexDst[1].bitangent);
			glm_vec3_dup(vertexDst[0].bitangent, vertexDst[2].bitangent);
		}

		vertexDst[0].mtl	= mesh->face_materials[faceIndex];
		vertexDst[1].mtl	= mesh->face_materials[faceIndex];
		vertexDst[2].mtl	= mesh->face_materials[faceIndex];

		*currentVertex += 3;
	}
}

static void	kdo_cpyVertex(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, fastObjMesh *mesh)
{
	uint32_t	currentFace;
	uint32_t	currentVertex;

	objectInfo->vertexCount		= kdo_triangleCount(mesh) * 3;

	if (!(objectInfo->vertex	= malloc(objectInfo->vertexCount * sizeof(Kdo_VkVertex))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(objectInfo->vec3Array = malloc(objectInfo->vertexCount * 3 * sizeof(vec3))))
		kdo_cleanup(vk, ERRLOC, 12);
	
	currentVertex	= 0;
	for (currentFace = 0; currentFace < mesh->face_count; currentFace++)
		kdo_fanTriangulation(objectInfo, mesh, &currentVertex, currentFace);
}

Kdo_VkObjectInfo	kdo_openObj(Kdo_Vulkan *vk, char *objPath)
{
	fastObjMesh			*mesh;
	Kdo_VkObjectInfo	objectInfo;

	mesh = fast_obj_read(objPath);
	
	kdo_cpyMaterial(vk, &objectInfo, mesh);	
	kdo_cpyVertex(vk, &objectInfo, mesh);

	fast_obj_destroy(mesh);
	return (objectInfo);
}
