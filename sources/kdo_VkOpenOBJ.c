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
	objectInfo->materialCount		= glm_max(mesh->material_count, 1);
	objectInfo->textureCount		= glm_max(mesh->material_count * 3, 1);

	if (!(objectInfo->material		= malloc(objectInfo->materialCount * sizeof(Kdo_ShMaterial))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(objectInfo->texturePath	= calloc(objectInfo->textureCount, sizeof(char *))))
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

static void kdo_findNormal(vec3 pos1, vec3 pos2, vec3 pos3, vec3 normal)
{
	vec3	x;
	vec3	y;

	glm_vec3_sub(pos2, pos3, x);
	glm_vec3_sub(pos2, pos1, y);
	glm_vec3_crossn(x, y, normal);
}

static void	kdo_findRawTangent(vec3 pos1, vec3 pos2, vec3 pos3, vec2 uv1, vec2 uv2, vec2 uv3, vec3 rawTangent)
{
	vec3	deltaPos1;
	vec3	deltaPos2;
	vec2	deltaUV1;
	vec2	deltaUV2;
	float	factor;

	glm_vec3_sub(pos2, pos1, deltaPos1);
	glm_vec3_sub(pos3, pos1, deltaPos2);
	glm_vec2_sub(uv2, uv1, deltaUV1);
	glm_vec2_sub(uv3, uv1, deltaUV2);
	factor = deltaUV1[0] * deltaUV2[1] - deltaUV1[1] * deltaUV2[0];

	glm_vec3_scale(deltaPos1, deltaUV2[1], deltaPos1);
	glm_vec3_scale(deltaPos2, deltaUV1[1], deltaPos1);
	glm_vec3_sub(deltaPos1, deltaPos2, rawTangent);
	glm_vec3_divs(rawTangent, factor, rawTangent);
}

static void	kdo_findTangent(vec3 rawTangent, vec2 normal, vec3 tangent)
{
	vec3			adjustVector;

	glm_vec3_proj(rawTangent, normal, adjustVector);
	glm_vec3_sub(rawTangent, adjustVector, tangent);
	glm_vec3_normalize(tangent);
}

static void	kdo_cpyTriangle(Kdo_VkRawVertex *vertexDst, fastObjMesh *mesh, uint32_t faceIndex, uint32_t vertexSrcIndex, uint32_t vertexSrcOffset)
{
	fastObjIndex	vertex1;
	fastObjIndex	vertex2;
	fastObjIndex	vertex3;
	vec3			rawTangent;

	vertex1	= mesh->indices[vertexSrcIndex];
	vertex2	= mesh->indices[vertexSrcIndex + vertexSrcOffset - 1];
	vertex3 = mesh->indices[vertexSrcIndex + vertexSrcOffset];

	glm_vec3_make(&mesh->positions[vertex1.p], vertexDst[0].pos);
	glm_vec3_make(&mesh->positions[vertex2.p], vertexDst[1].pos);
	glm_vec3_make(&mesh->positions[vertex3.p], vertexDst[2].pos);

	glm_vec2_make(&mesh->texcoords[vertex1.t], vertexDst[0].uv);
	glm_vec2_make(&mesh->texcoords[vertex2.t], vertexDst[1].uv);
	glm_vec2_make(&mesh->texcoords[vertex3.t], vertexDst[2].uv);

	kdo_findRawTangent(vertexDst[0].pos, vertexDst[1].pos, vertexDst[2].pos, vertexDst[0].uv, vertexDst[1].uv, vertexDst[2].uv, rawTangent);
	if (1 < mesh->normal_count)
	{
		glm_vec3_make(&mesh->normals[vertex1.n], vertexDst[0].normal);
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
}

static void	kdo_cpyVertex(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, fastObjMesh *mesh)
{
	uint32_t	currentFace;
	uint32_t	vertexSrcOffset;
	uint32_t	currentVertexSrc;
	uint32_t	currentVertexDst;

	objectInfo->vertexCount		= kdo_triangleCount(mesh) * 3;

	if (!(objectInfo->vertex	= malloc(objectInfo->vertexCount * sizeof(Kdo_VkRawVertex))))
		kdo_cleanup(vk, ERRLOC, 12);
	
	currentVertexSrc = 0;
	currentVertexDst = 0;
	for (currentFace = 0; currentFace < mesh->face_count; currentFace++)
	{
		for (vertexSrcOffset = 2; vertexSrcOffset < mesh->face_vertices[currentFace]; vertexSrcOffset++)
		{
			kdo_cpyTriangle(&objectInfo->vertex[currentVertexDst], mesh, currentFace, currentVertexSrc, vertexSrcOffset);	
			currentVertexDst += 3;
		}
		currentVertexSrc += mesh->face_vertices[currentFace];
	}
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
