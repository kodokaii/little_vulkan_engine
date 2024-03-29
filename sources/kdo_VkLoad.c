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

#include "kdo_VkLoad.h"
#include "kdo_VkOpenOBJ.h"

static void	kdo_updateTextureDescriptor(Kdo_Vulkan *vk)
{
	VkDescriptorImageInfo			writeImageInfo[MAX_TEXTURE];
	VkWriteDescriptorSet			descriptorWrite;

	for (uint32_t i = 0; i < MAX_TEXTURE; i++)
	{
		writeImageInfo[i].sampler		= NULL;
		if (i < kdo_getGPUImageCount(vk->core.buffer.texture))
		{
			writeImageInfo[i].imageView		= kdo_getGPUImage(vk->core.buffer.texture, i).view;
			writeImageInfo[i].imageLayout	= kdo_getGPUImage(vk->core.buffer.texture, i).layout;
		}
		else
		{
			writeImageInfo[i].imageView		= kdo_getGPUImage(vk->core.buffer.texture, 0).view;
			writeImageInfo[i].imageLayout	= kdo_getGPUImage(vk->core.buffer.texture, 0).layout;
		}
	}

	descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext				= NULL;
	descriptorWrite.dstSet				= vk->core.descriptorSet;
	descriptorWrite.dstBinding			= 4;
	descriptorWrite.dstArrayElement		= 0;
	descriptorWrite.descriptorCount		= MAX_TEXTURE;
	descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorWrite.pImageInfo			= writeImageInfo;
	descriptorWrite.pBufferInfo			= NULL;
	descriptorWrite.pTexelBufferView	= NULL;
	vkUpdateDescriptorSets(vk->device.path, 1, &descriptorWrite, 0, NULL);
}

static void kdo_updateDescriptor(Kdo_Vulkan *vk)
{
	VkDescriptorBufferInfo			writeBufferInfo[5];
	VkWriteDescriptorSet			descriptorWrite[5];

	writeBufferInfo[0].buffer	= kdo_getGPUBuffer(vk->core.buffer.object);
	writeBufferInfo[0].offset	= 0;
	writeBufferInfo[0].range	= kdo_getGPUBufferSize(vk->core.buffer.object);

	writeBufferInfo[1].buffer	= kdo_getGPUBuffer(vk->core.buffer.vector4);
	writeBufferInfo[1].offset	= 0;
	writeBufferInfo[1].range	= kdo_getGPUBufferSize(vk->core.buffer.vector4);

	writeBufferInfo[2].buffer	= kdo_getGPUBuffer(vk->core.buffer.vector2);
	writeBufferInfo[2].offset	= 0;
	writeBufferInfo[2].range	= kdo_getGPUBufferSize(vk->core.buffer.vector2);

	writeBufferInfo[3].buffer	= kdo_getGPUBuffer(vk->core.buffer.material);
	writeBufferInfo[3].offset	= 0;
	writeBufferInfo[3].range	= kdo_getGPUBufferSize(vk->core.buffer.material);

	writeBufferInfo[4].buffer	= kdo_getGPUBuffer(vk->core.buffer.light);
	writeBufferInfo[4].offset	= 0;
	writeBufferInfo[4].range	= kdo_getGPUBufferSize(vk->core.buffer.light);


	descriptorWrite[0].sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[0].pNext               = NULL;
	descriptorWrite[0].dstSet              = vk->core.descriptorSet;
	descriptorWrite[0].dstBinding          = 0;
	descriptorWrite[0].dstArrayElement     = 0;
	descriptorWrite[0].descriptorCount     = 1;
	descriptorWrite[0].descriptorType      = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite[0].pImageInfo          = NULL;
	descriptorWrite[0].pBufferInfo         = writeBufferInfo;
	descriptorWrite[0].pTexelBufferView    = NULL;

	descriptorWrite[1].sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[1].pNext               = NULL;
	descriptorWrite[1].dstSet              = vk->core.descriptorSet;
	descriptorWrite[1].dstBinding          = 1;
	descriptorWrite[1].dstArrayElement     = 0;
	descriptorWrite[1].descriptorCount     = 1;
	descriptorWrite[1].descriptorType      = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite[1].pImageInfo          = NULL;
	descriptorWrite[1].pBufferInfo         = writeBufferInfo + 1;
	descriptorWrite[1].pTexelBufferView    = NULL;

	descriptorWrite[2].sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[2].pNext               = NULL;
	descriptorWrite[2].dstSet              = vk->core.descriptorSet;
	descriptorWrite[2].dstBinding          = 2;
	descriptorWrite[2].dstArrayElement     = 0;
	descriptorWrite[2].descriptorCount     = 1;
	descriptorWrite[2].descriptorType      = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite[2].pImageInfo          = NULL;
	descriptorWrite[2].pBufferInfo         = writeBufferInfo + 2;
	descriptorWrite[2].pTexelBufferView    = NULL;

	descriptorWrite[3].sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[3].pNext               = NULL;
	descriptorWrite[3].dstSet              = vk->core.descriptorSet;
	descriptorWrite[3].dstBinding          = 5;
	descriptorWrite[3].dstArrayElement     = 0;
	descriptorWrite[3].descriptorCount     = 1;
	descriptorWrite[3].descriptorType      = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite[3].pImageInfo          = NULL;
	descriptorWrite[3].pBufferInfo         = writeBufferInfo + 3;
	descriptorWrite[3].pTexelBufferView    = NULL;

	descriptorWrite[4].sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[4].pNext               = NULL;
	descriptorWrite[4].dstSet              = vk->core.descriptorSet;
	descriptorWrite[4].dstBinding          = 6;
	descriptorWrite[4].dstArrayElement     = 0;
	descriptorWrite[4].descriptorCount     = 1;
	descriptorWrite[4].descriptorType      = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite[4].pImageInfo          = NULL;
	descriptorWrite[4].pBufferInfo         = writeBufferInfo + 4;
	descriptorWrite[4].pTexelBufferView    = NULL;

	vkUpdateDescriptorSets(vk->device.path, 5, descriptorWrite, 0, NULL);

	kdo_updateTextureDescriptor(vk);
}

static uint32_t	kdo_calculateNormal(Kdo_Vulkan *vk, Kdo_VkVertex vertex[3])
{
	vec3		pos1;
	vec3		pos2;
	vec3		pos3;
	vec3		deltaPos1;
	vec3		deltaPos2;
	vec4		normal;
	uint32_t	index;

	glm_vec3(((vec4 *)kdo_getCPUBuffer(vk->core.buffer.vector4))[vertex[0].posIndex], pos1);
	glm_vec3(((vec4 *)kdo_getCPUBuffer(vk->core.buffer.vector4))[vertex[1].posIndex], pos2);
	glm_vec3(((vec4 *)kdo_getCPUBuffer(vk->core.buffer.vector4))[vertex[2].posIndex], pos3);

	glm_vec3_sub(pos2, pos1, deltaPos1);
	glm_vec3_sub(pos3, pos1, deltaPos2);
	glm_vec3_crossn(deltaPos1, deltaPos2, normal);

	normal[3] = 0;
	kdo_vkPushSetBufferData(vk, &vk->core.buffer.vector4, normal, &index);

	return (index);
}

static void	kdo_calculateRawTangent(Kdo_Vulkan *vk, Kdo_VkVertex vertex[3], vec3 rawTangent)
{
	vec3	pos1;
	vec3	pos2;
	vec3	pos3;

	vec3	uv1;
	vec3	uv2;
	vec3	uv3;
	vec3    deltaPos1;
	vec3    deltaPos2;
	vec2    deltaUV1;
	vec2    deltaUV2;
	float   factor;
	
	glm_vec3(((vec4 *)kdo_getCPUBuffer(vk->core.buffer.vector4))[vertex[0].posIndex], pos1);
	glm_vec3(((vec4 *)kdo_getCPUBuffer(vk->core.buffer.vector4))[vertex[1].posIndex], pos2);
	glm_vec3(((vec4 *)kdo_getCPUBuffer(vk->core.buffer.vector4))[vertex[2].posIndex], pos3);
	
	glm_vec2_copy(((vec2 *)kdo_getCPUBuffer(vk->core.buffer.vector2))[vertex[0].uvIndex], uv1);
	glm_vec2_copy(((vec2 *)kdo_getCPUBuffer(vk->core.buffer.vector2))[vertex[1].uvIndex], uv2);
	glm_vec2_copy(((vec2 *)kdo_getCPUBuffer(vk->core.buffer.vector2))[vertex[2].uvIndex], uv3);

	glm_vec3_sub(pos2, pos1, deltaPos1);
	glm_vec3_sub(pos3, pos1, deltaPos2);
	glm_vec2_sub(uv2, uv1, deltaUV1);
	glm_vec2_sub(uv3, uv1, deltaUV2);
	factor = deltaUV1[0] * deltaUV2[1] - deltaUV1[1] * deltaUV2[0];

	glm_vec3_scale(deltaPos1, deltaUV2[1], deltaPos1);
	glm_vec3_scale(deltaPos2, deltaUV1[1], deltaPos2);
	glm_vec3_sub(deltaPos1, deltaPos2, rawTangent);
	glm_vec3_divs(rawTangent, factor, rawTangent);
}

static uint32_t	kdo_calculateTangent(Kdo_Vulkan *vk, Kdo_VkVertex vertex, vec3 rawTangent)
{
	vec3			normal;
	vec3            adjustVector;
	vec4			tangent;
	uint32_t		index;
	glm_vec3(((vec4 *)kdo_getCPUBuffer(vk->core.buffer.vector4))[vertex.normalIndex], normal);

	glm_vec3_proj(rawTangent, normal, adjustVector);
	glm_vec3_sub(rawTangent, adjustVector, tangent);
	glm_vec3_normalize(tangent);

	tangent[3]	= 0;
	kdo_vkPushSetBufferData(vk, &vk->core.buffer.vector4, tangent, &index);

	return (index);
}

static uint32_t	kdo_calculateBitangent(Kdo_Vulkan *vk, Kdo_VkVertex vertex)
{
	vec3		normal;
	vec3		tangent;
	vec4		bitangent;
	uint32_t	index;

	glm_vec3(((vec4 *)kdo_getCPUBuffer(vk->core.buffer.vector4))[vertex.normalIndex], normal);	
	glm_vec3(((vec4 *)kdo_getCPUBuffer(vk->core.buffer.vector4))[vertex.tangentIndex], tangent);	

	glm_vec3_crossn(normal, tangent, bitangent);

	bitangent[3] = 0;
	kdo_vkPushSetBufferData(vk, &vk->core.buffer.vector4, bitangent, &index);

	return (index);
}

VkResult	kdo_initObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t maxVertexCount, uint32_t maxPosCount, uint32_t maxTangentCount , uint32_t maxBitangentCount, uint32_t maxNormalCount, uint32_t maxUvCount, uint32_t maxMaterialCount, uint32_t maxTextureCount)
{
	if (!maxVertexCount || !maxPosCount)
		return (VK_ERROR_NOT_PERMITTED_KHR);

	KDO_VK_ALLOC(objectInfo->vertex, calloc(maxVertexCount, sizeof (Kdo_VkVertex)));
	KDO_VK_ALLOC(objectInfo->pos.matchArray, calloc(maxPosCount + 1, sizeof (uint32_t)));
	KDO_VK_ALLOC(objectInfo->tangent.matchArray, calloc(maxTangentCount + 1, sizeof (uint32_t)));
	KDO_VK_ALLOC(objectInfo->bitangent.matchArray, calloc(maxBitangentCount + 1, sizeof (uint32_t)));
	KDO_VK_ALLOC(objectInfo->normal.matchArray, calloc(maxNormalCount + 1, sizeof (uint32_t)));
	KDO_VK_ALLOC(objectInfo->uv.matchArray, calloc(maxUvCount + 1, sizeof (uint32_t)));
	KDO_VK_ALLOC(objectInfo->material.matchArray, calloc(maxMaterialCount + 1, sizeof (uint32_t)));
	KDO_VK_ALLOC(objectInfo->texture.matchArray, calloc(maxTextureCount + 1, sizeof (uint32_t)));

	glm_mat4_identity(objectInfo->object.modelMat);
	glm_mat4_identity(objectInfo->object.normalMat);
	objectInfo->pos.maxCount						= maxPosCount;
	objectInfo->tangent.maxCount					= maxTangentCount;
	objectInfo->bitangent.maxCount					= maxBitangentCount;
	objectInfo->normal.maxCount						= maxNormalCount;
	objectInfo->uv.maxCount							= maxUvCount;
	objectInfo->material.maxCount					= maxMaterialCount;
	objectInfo->texture.maxCount					= maxTextureCount;
	objectInfo->vertexMaxCount						= maxVertexCount;
	objectInfo->vertexCount							= 0;

	return (VK_SUCCESS);
}

VkResult	kdo_addPosObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec4 pos)
{
	uint32_t	matchIndex;

	if (!index || objectInfo->pos.maxCount < index)
		return (VK_ERROR_NOT_PERMITTED_KHR);

	kdo_vkPushSetBufferData(vk, &vk->core.buffer.vector4, pos, &matchIndex);
	
	objectInfo->pos.matchArray[index] = matchIndex;

	return (VK_SUCCESS);
}

VkResult	kdo_addTangentObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec4 tangent)
{
	uint32_t	matchIndex;

	if (!index || objectInfo->tangent.maxCount < index)
		return (VK_ERROR_NOT_PERMITTED_KHR);

	glm_vec3_normalize(tangent);
	kdo_vkPushSetBufferData(vk, &vk->core.buffer.vector4, tangent, &matchIndex);
	
	objectInfo->tangent.matchArray[index] = matchIndex;

	return (VK_SUCCESS);
}

VkResult	kdo_addBitangentObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec4 bitangent)
{
	uint32_t	matchIndex;

	if (!index || objectInfo->bitangent.maxCount < index)
		return (VK_ERROR_NOT_PERMITTED_KHR);

	glm_vec3_normalize(bitangent);
	kdo_vkPushSetBufferData(vk, &vk->core.buffer.vector4, bitangent, &matchIndex);
	
	objectInfo->bitangent.matchArray[index] = matchIndex;

	return (VK_SUCCESS);
}

VkResult	kdo_addNormalObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec4 normal)
{
	uint32_t	matchIndex;

	if (!index || objectInfo->normal.maxCount < index)
		return (VK_ERROR_NOT_PERMITTED_KHR);

	glm_vec3_normalize(normal);
	kdo_vkPushSetBufferData(vk, &vk->core.buffer.vector4, normal, &matchIndex);
	
	objectInfo->normal.matchArray[index] = matchIndex;

	return (VK_SUCCESS);
}

VkResult	kdo_addUvObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec2 uv)
{
	uint32_t	matchIndex;

	if (!index || objectInfo->uv.maxCount < index)
		return (VK_ERROR_NOT_PERMITTED_KHR);

	uv[1]	= 1 - uv[1];
	kdo_vkPushSetBufferData(vk, &vk->core.buffer.vector2, uv, &matchIndex);
	
	objectInfo->uv.matchArray[index] = matchIndex;

	return (VK_SUCCESS);
}

VkResult	kdo_addTextureObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, char *path)
{
	uint32_t	matchIndex;

	if (!index || objectInfo->texture.maxCount < index)
		return (VK_ERROR_NOT_PERMITTED_KHR);

	kdo_vkPushSetImageBufferPath(vk, &vk->core.buffer.texture, path, &matchIndex);
	
	objectInfo->texture.matchArray[index] = matchIndex;

	return (VK_SUCCESS);
}

VkResult	kdo_addMaterialObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, Kdo_VkMaterial *materialIn)
{
	Kdo_VkMaterial	materialOut;
	uint32_t		matchIndex;

	if (!index || objectInfo->material.maxCount < index)
		return (VK_ERROR_NOT_PERMITTED_KHR);

	glm_vec3_copy(materialIn->ambient, materialOut.ambient);
	glm_vec3_copy(materialIn->diffuse, materialOut.diffuse);
	glm_vec3_copy(materialIn->specular, materialOut.specular);
	glm_vec3_copy(materialIn->emissive, materialOut.emissive);
	glm_vec3_copy(materialIn->transmittance, materialOut.transmittance);
	glm_vec3_copy(materialIn->transmissionFilter, materialOut.transmissionFilter);
	materialOut.shininess				= materialIn->shininess;
	materialOut.refractionIndex			= materialIn->refractionIndex;
	materialOut.disolve					= materialIn->disolve;
	materialOut.illum					= materialIn->illum;
	materialOut.ambientMap				= objectInfo->texture.matchArray[materialIn->ambientMap];
	materialOut.diffuseMap				= objectInfo->texture.matchArray[materialIn->diffuseMap];
	materialOut.specularMap				= objectInfo->texture.matchArray[materialIn->specularMap];
	materialOut.emissiveMap				= objectInfo->texture.matchArray[materialIn->emissiveMap];
	materialOut.transmittanceMap		= objectInfo->texture.matchArray[materialIn->transmittanceMap];
	materialOut.transmissionFilterMap	= objectInfo->texture.matchArray[materialIn->transmissionFilterMap];
	materialOut.shininessMap			= objectInfo->texture.matchArray[materialIn->shininessMap];
	materialOut.disolveMap				= objectInfo->texture.matchArray[materialIn->disolveMap];
	materialOut.bumpMap					= objectInfo->texture.matchArray[materialIn->bumpMap];

	kdo_vkPushSetBufferData(vk, &vk->core.buffer.material, &materialOut, &matchIndex);
	
	objectInfo->material.matchArray[index] = matchIndex;

	return (VK_SUCCESS);
}

VkResult	kdo_addTriangleObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, Kdo_VkVertex vertexIn[3])
{
	Kdo_VkVertex	*vertexOut;	
	int				validUv;
	vec3			rawTangent;

	vertexOut	= objectInfo->vertex + objectInfo->vertexCount;

	if (!vertexIn[0].posIndex || !vertexIn[1].posIndex || !vertexIn[2].posIndex)
		return (VK_ERROR_NOT_PERMITTED_KHR);

	vertexOut[0].posIndex		= objectInfo->pos.matchArray[vertexIn[0].posIndex];
	vertexOut[1].posIndex		= objectInfo->pos.matchArray[vertexIn[1].posIndex];
	vertexOut[2].posIndex		= objectInfo->pos.matchArray[vertexIn[2].posIndex];


	validUv = vertexIn[0].uvIndex && vertexIn[1].uvIndex && vertexIn[2].uvIndex;

	vertexOut[0].uvIndex		= objectInfo->uv.matchArray[vertexIn[0].uvIndex];
	vertexOut[1].uvIndex		= objectInfo->uv.matchArray[vertexIn[1].uvIndex];
	vertexOut[2].uvIndex		= objectInfo->uv.matchArray[vertexIn[2].uvIndex];


	if (!vertexIn[0].normalIndex || !vertexIn[1].normalIndex || !vertexIn[2].normalIndex)
		objectInfo->normal.matchArray[0] = kdo_calculateNormal(vk, vertexOut);

	vertexOut[0].normalIndex	= objectInfo->normal.matchArray[vertexIn[0].normalIndex];
	vertexOut[1].normalIndex	= objectInfo->normal.matchArray[vertexIn[1].normalIndex];
	vertexOut[2].normalIndex	= objectInfo->normal.matchArray[vertexIn[2].normalIndex];


	if (validUv && (!vertexIn[0].tangentIndex || !vertexIn[1].tangentIndex || !vertexIn[2].tangentIndex))
		kdo_calculateRawTangent(vk, vertexOut, rawTangent);

	if (validUv && !vertexIn[0].tangentIndex)
		vertexOut[0].tangentIndex   = kdo_calculateTangent(vk, vertexOut[0], rawTangent);
	else
		vertexOut[0].tangentIndex   = objectInfo->tangent.matchArray[vertexIn[0].tangentIndex];
	if (validUv && !vertexIn[1].tangentIndex)
		vertexOut[1].tangentIndex   = kdo_calculateTangent(vk, vertexOut[1], rawTangent);
	else
		vertexOut[1].tangentIndex   = objectInfo->tangent.matchArray[vertexIn[1].tangentIndex];
	if (validUv && !vertexIn[2].tangentIndex)
		vertexOut[2].tangentIndex   = kdo_calculateTangent(vk, vertexOut[2], rawTangent);
	else
		vertexOut[2].tangentIndex   = objectInfo->tangent.matchArray[vertexIn[2].tangentIndex];


	if (!vertexIn[0].bitangentIndex)
		 vertexOut[0].bitangentIndex	= kdo_calculateBitangent(vk, vertexOut[0]);
	else
		vertexOut[0].bitangentIndex	=  objectInfo->bitangent.matchArray[vertexIn[0].bitangentIndex];
	if (!vertexIn[1].bitangentIndex)
		 vertexOut[1].bitangentIndex	= kdo_calculateBitangent(vk, vertexOut[1]);
	else
		vertexOut[1].bitangentIndex	=  objectInfo->bitangent.matchArray[vertexIn[1].bitangentIndex];
	if (!vertexIn[2].bitangentIndex)
		 vertexOut[2].bitangentIndex	= kdo_calculateBitangent(vk, vertexOut[2]);
	else
		vertexOut[2].bitangentIndex	=  objectInfo->bitangent.matchArray[vertexIn[2].bitangentIndex];


	vertexOut[0].mtlIndex	= objectInfo->material.matchArray[vertexIn[0].mtlIndex];
	vertexOut[1].mtlIndex	= objectInfo->material.matchArray[vertexIn[1].mtlIndex];
	vertexOut[2].mtlIndex	= objectInfo->material.matchArray[vertexIn[2].mtlIndex];

	objectInfo->vertexCount += 3;

	return (VK_SUCCESS);
}

VkResult	kdo_openObject(Kdo_Vulkan *vk, char *objectPath, Kdo_VkObjectInfo *objectInfo)
{
	char	*extension;
	
	extension = strrchr(objectPath, '.');
	if (!extension)
		return (VK_ERROR_UNKNOWN);

	if (!strcmp(extension, ".obj"))
		return (kdo_openObj(vk, objectInfo, objectPath));

	return (VK_ERROR_UNKNOWN);
}

VkResult	kdo_loadObject(Kdo_Vulkan *vk, uint32_t objectCount, Kdo_VkObjectInfo *info)
{
	uint32_t	i;

	if (!objectCount)
		return (VK_SUCCESS);

	for (i = 0; i < objectCount; i++)
	{
		info[i].object.drawCommand.vertexCount		= info[i].vertexCount;
		info[i].object.drawCommand.instanceCount	= 1;
		info[i].object.drawCommand.firstVertex		= kdo_getCPUBufferSize(vk->core.buffer.vertex) / sizeof(Kdo_VkVertex);
		info[i].object.drawCommand.firstInstance	= vk->core.objectCount + i;

		kdo_vkPushBufferData(vk, &vk->core.buffer.vertex, info[i].vertex, info[i].vertexCount * sizeof(Kdo_VkVertex));
		kdo_vkPushBufferData(vk, &vk->core.buffer.object, &info[i].object,  sizeof(Kdo_VkObject));

		KDO_FREE(info[i].vertex);
		KDO_FREE(info[i].pos.matchArray);
		KDO_FREE(info[i].tangent.matchArray);
		KDO_FREE(info[i].bitangent.matchArray);
		KDO_FREE(info[i].normal.matchArray);
		KDO_FREE(info[i].uv.matchArray);
		KDO_FREE(info[i].material.matchArray);
		KDO_FREE(info[i].texture.matchArray);
	}

	kdo_vkUpdateSetBuffer(vk, &vk->core.buffer.vector4);
	kdo_vkUpdateSetBuffer(vk, &vk->core.buffer.vector2);
	kdo_vkUpdateSetBuffer(vk, &vk->core.buffer.material);
	kdo_vkUpdateSetImageBuffer(vk, &vk->core.buffer.texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	kdo_vkUpdateAllBuffer(vk, &vk->core.buffer.vertex);
	kdo_vkUpdateAllBuffer(vk, &vk->core.buffer.object);

	kdo_updateDescriptor(vk);

	vk->core.objectCount	+= objectCount;

	return (VK_SUCCESS);
}
