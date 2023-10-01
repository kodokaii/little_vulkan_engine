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

Kdo_VkMaterial	kdo_defaultMaterial(void)
{
	Kdo_VkMaterial	defaultMtl	=	{{0, 0, 0}, 0,	//ambient
									{1, 1, 1}, 0,	//diffuse
									{1, 1, 1}, 0,	//specular
									{1, 1, 1}, 0,	//emissive
									{1, 1, 1}, 0,	//transmittance
									{1, 1, 1}, 0,	//transmissionFilter
									400, 0,			//shininess
									1, 0,			//refractionIndex
									1, 0,			//disolve
									1,				//illum
									0};				//bumpMap
	return (defaultMtl);
}

static void	kdo_updateAllTextureDescriptor(Kdo_Vulkan *vk)
{
	VkDescriptorImageInfo			writeImageInfo[MAX_TEXTURES]; VkWriteDescriptorSet			descriptorWrite;

	for (uint32_t i = 0; i < MAX_TEXTURES; i++)
	{
		writeImageInfo[i].sampler		= NULL;
		if (i < vk->core.buffer.textures.imageCount)
			writeImageInfo[i].imageView		= vk->core.buffer.textures.image[i].view;
		else
			writeImageInfo[i].imageView		= vk->core.buffer.textures.image[0].view;
		writeImageInfo[i].imageLayout	= vk->core.buffer.textures.properties.layout;
	}

	descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext				= NULL;
	descriptorWrite.dstSet				= vk->core.descriptorSet;
	descriptorWrite.dstBinding			= 3;
	descriptorWrite.dstArrayElement		= 0;
	descriptorWrite.descriptorCount		= MAX_TEXTURES;
	descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorWrite.pImageInfo			= writeImageInfo;
	descriptorWrite.pBufferInfo			= NULL;
	descriptorWrite.pTexelBufferView	= NULL;
	vkUpdateDescriptorSets(vk->device.path, 1, &descriptorWrite, 0, NULL); }

static void kdo_initDescriptorSets(Kdo_Vulkan *vk)
{
	VkWriteDescriptorSet			descriptorWrite[4];
	VkDescriptorBufferInfo			writeBufferInfo[4];

	writeBufferInfo[0].buffer	= vk->core.buffer.object.buffer;
	writeBufferInfo[0].offset	= 0;
	writeBufferInfo[0].range	= vk->core.buffer.object.sizeFree;

	writeBufferInfo[1].buffer	= vk->core.buffer.materialMap.buffer;
	writeBufferInfo[1].offset	= 0;
	writeBufferInfo[1].range	= vk->core.buffer.materialMap.sizeFree;

	writeBufferInfo[2].buffer	= vk->core.buffer.light.buffer;
	writeBufferInfo[2].offset	= 0;
	writeBufferInfo[2].range	= vk->core.buffer.light.sizeFree;

	writeBufferInfo[3].buffer	= vk->core.buffer.materials.buffer;
	writeBufferInfo[3].offset	= 0;
	writeBufferInfo[3].range	= vk->core.buffer.materials.sizeFree;


	descriptorWrite[0].sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[0].pNext               = NULL;
	descriptorWrite[0].dstSet              = vk->core.descriptorSet;
	descriptorWrite[0].dstBinding          = 0;
	descriptorWrite[0].dstArrayElement     = 0;
	descriptorWrite[0].descriptorCount     = 1;
	descriptorWrite[0].descriptorType      = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[0].pImageInfo          = NULL;
	descriptorWrite[0].pBufferInfo         = writeBufferInfo;
	descriptorWrite[0].pTexelBufferView    = NULL;

	descriptorWrite[1].sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[1].pNext               = NULL;
	descriptorWrite[1].dstSet              = vk->core.descriptorSet;
	descriptorWrite[1].dstBinding          = 1;
	descriptorWrite[1].dstArrayElement     = 0;
	descriptorWrite[1].descriptorCount     = 1;
	descriptorWrite[1].descriptorType      = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[1].pImageInfo          = NULL;
	descriptorWrite[1].pBufferInfo         = writeBufferInfo + 1;
	descriptorWrite[1].pTexelBufferView    = NULL;

	descriptorWrite[2].sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[2].pNext               = NULL;
	descriptorWrite[2].dstSet              = vk->core.descriptorSet;
	descriptorWrite[2].dstBinding          = 4;
	descriptorWrite[2].dstArrayElement     = 0;
	descriptorWrite[2].descriptorCount     = 1;
	descriptorWrite[2].descriptorType      = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[2].pImageInfo          = NULL;
	descriptorWrite[2].pBufferInfo         = writeBufferInfo + 2;
	descriptorWrite[2].pTexelBufferView    = NULL;

	descriptorWrite[3].sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[3].pNext               = NULL;
	descriptorWrite[3].dstSet              = vk->core.descriptorSet;
	descriptorWrite[3].dstBinding          = 5;
	descriptorWrite[3].dstArrayElement     = 0;
	descriptorWrite[3].descriptorCount     = 1;
	descriptorWrite[3].descriptorType      = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[3].pImageInfo          = NULL;
	descriptorWrite[3].pBufferInfo         = writeBufferInfo + 3;
	descriptorWrite[3].pTexelBufferView    = NULL;
	vkUpdateDescriptorSets(vk->device.path, 4, descriptorWrite, 0, NULL);


	if (kdo_loadTexture(vk, DEFAULT_TEXTURE))
		kdo_cleanup(vk, "Defautl textures load failed", 34);

	kdo_updateAllTextureDescriptor(vk);
}

static uint32_t	kdo_loadTexture(Kdo_Vulkan *vk, char *texturePath)
{
	VkDescriptorImageInfo	writeImageInfo;
	VkWriteDescriptorSet	descriptorWrite;
	VkDeviceSize			oldSize;
	Kdo_VkImageFuncInfo		funcInfo = {kdo_imageTextureInfo, kdo_viewTextureInfo};

	oldSize = vk->core.buffer.textures.sizeFree + vk->core.buffer.textures.sizeUsed;
	if (!texturePath || kdo_appendImage(vk, &vk->core.buffer.textures, texturePath, funcInfo))
		return (1);

	if (oldSize != vk->core.buffer.textures.sizeFree + vk->core.buffer.textures.sizeUsed)
		kdo_updateAllTextureDescriptor(vk);
	else
	{
		writeImageInfo.sampler				= NULL;
		writeImageInfo.imageView			= vk->core.buffer.textures.image[vk->core.buffer.textures.imageCount - 1].view;
		writeImageInfo.imageLayout			= vk->core.buffer.textures.properties.layout;

		descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.pNext				= NULL;
		descriptorWrite.dstSet				= vk->core.descriptorSet;
		descriptorWrite.dstBinding			= 3;
		descriptorWrite.dstArrayElement		= vk->core.buffer.textures.imageCount - 1;
		descriptorWrite.descriptorCount		= 1;
		descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		descriptorWrite.pImageInfo			= &writeImageInfo;
		descriptorWrite.pBufferInfo			= NULL;
		descriptorWrite.pTexelBufferView	= NULL;
		vkUpdateDescriptorSets(vk->device.path, 1, &descriptorWrite, 0, NULL);
	}

	return (0);
}

static int	kdo_allocBufferCPU(Kdo_VkBufferCoreElement *buffer, size_t needSize)
{
	if (buffer->sizeFree < needSize)
		if (!(buffer->bufferCPU	= realloc(buffer->bufferCPU, buffer->sizeUsed + needSize)))
			return (1);
	buffer->sizeUsed	+= needSize;
	buffer->sizeFree	-= glm_min(buffer->sizeFree, needSize);

	return (0);
}

static void	kdo_freeBufferCPU(Kdo_VkBufferCoreElement *buffer, size_t freeSize)
{
	buffer->sizeFree	+= glm_min(buffer->sizeUsed, freeSize);	
	buffer->sizeUsed	-= glm_min(buffer->sizeUsed, freeSize);
}

static int	kdo_allocImageCPU(Kdo_VkImageCoreElement *image, uint32_t needCount)
{
	if (image->countFree < needCount)
		if (!(image->nameCPU	= realloc(image->nameCPU, (image->countUsed + needCount) * sizeof(char *))))
			return (1);
	image->countUsed	+= needCount;
	image->countFree	-= glm_min(image->countFree, needCount);

	return (0);
}

static void	kdo_freeImageCPU(Kdo_VkImageCoreElement *image, uint32_t freeCount)
{
	image->countFree	+= glm_min(image->countUsed, freeCount);	
	image->countUsed	-= glm_min(image->countUsed, freeCount);
}

static int	kdo_addDataCPU(Kdo_VkBufferCoreElement *buffer, size_t sizeData, void *data, uint32_t *index)
{	
	uint32_t	endIndex;

	endIndex	= buffer->sizeFill / sizeData;
	*index		= endIndex;	
	if (!(kdo_BSTaddData(&buffer->BSTRoot, buffer->bufferCPU, sizeData, index, data)))
		return (-1);
	if (*index == endIndex)
	{
		buffer->sizeFill += sizeData;
		return (1);
	}
	return (0);
}

static int	kdo_addImageCPU(Kdo_VkImageCoreElement *image, char *path, uint32_t *index)
{	
	uint32_t	endIndex;

	endIndex	= image->countFill;
	*index		= endIndex;	
	if (!(kdo_BSTaddStr(&image->BSTRoot, image->nameCPU, index, basename(path))))
		return (-1);
	if (*index == endIndex)
	{
		image->countFill++;
		return (1);
	}
	return (0);
}

static uint32_t	kdo_calculateNormal(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, Kdo_VkVertex vertex[3])
{
	vec3		pos1;
	vec3		pos2;
	vec3		pos3;
	vec3		deltaPos1;
	vec3		deltaPos2;
	vec3		normal;
	int			returnCode;
	uint32_t	index;

	glm_vec3_copy(((vec3 *)vk->core.buffer.vector3.bufferCPU)[vertex[0].posIndex], pos1);
	glm_vec3_copy(((vec3 *)vk->core.buffer.vector3.bufferCPU)[vertex[1].posIndex], pos2);
	glm_vec3_copy(((vec3 *)vk->core.buffer.vector3.bufferCPU)[vertex[2].posIndex], pos3);

	glm_vec3_sub(pos2, pos1, deltaPos1);
	glm_vec3_sub(pos3, pos1, deltaPos2);
	glm_vec3_crossn(deltaPos1, deltaPos2, normal);

	if (!objectInfo->normal.freeCount)
		return (0);

	returnCode	= kdo_addDataCPU(vk->core.buffer.vector3.bufferCPU, sizeof(vec3), &normal, &index);
	if (returnCode == -1)
		kdo_cleanup(vk, ERRLOC, 12);
	if (returnCode == 1)
		objectInfo->normal.freeCount--;

	return (index);
}

static void	kdo_calculateRawTangent(Kdo_Vulkan *vk, Kdo_VkVertex vertex[3], vec3 rawTangent)
{
	vec3	pos1;
	vec3	pos2;
	vec3	pos3;
	vec2	uv1;
	vec2	uv2;
	vec2	uv3;
	vec3    deltaPos1;
	vec3    deltaPos2;
	vec2    deltaUV1;
	vec2    deltaUV2;
	float   factor;
	
	glm_vec3_copy(((vec3 *)vk->core.buffer.vector3.bufferCPU)[vertex[0].posIndex], pos1);
	glm_vec3_copy(((vec3 *)vk->core.buffer.vector3.bufferCPU)[vertex[1].posIndex], pos2);
	glm_vec3_copy(((vec3 *)vk->core.buffer.vector3.bufferCPU)[vertex[2].posIndex], pos3);
	
	glm_vec2_copy(((vec2 *)vk->core.buffer.vector2.bufferCPU)[vertex[0].uvIndex], uv1);
	glm_vec2_copy(((vec2 *)vk->core.buffer.vector2.bufferCPU)[vertex[1].uvIndex], uv2);
	glm_vec2_copy(((vec2 *)vk->core.buffer.vector2.bufferCPU)[vertex[2].uvIndex], uv3);

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

static uint32_t	kdo_calculateTangent(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, Kdo_VkVertex vertex, vec3 rawTangent)
{
	vec3			normal;
	vec3            adjustVector;
	vec3			tangent;
	int				returnCode;
	uint32_t		index;

	glm_vec3_copy(((vec3 *)vk->core.buffer.vector3.bufferCPU)[vertex.normalIndex], normal);

	glm_vec3_proj(rawTangent, normal, adjustVector);
	glm_vec3_sub(rawTangent, adjustVector, tangent);
	glm_vec3_normalize(tangent);

	if (!objectInfo->tangent.freeCount)
		return (0);

	returnCode	= kdo_addDataCPU(vk->core.buffer.vector3.bufferCPU, sizeof(vec3), &tangent, &index);
	if (returnCode == -1)
		kdo_cleanup(vk, ERRLOC, 12);
	if (returnCode == 1)
		objectInfo->tangent.freeCount--;

	return (index);
}

static uint32_t	kdo_calculateBitangent(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, Kdo_VkVertex vertex)
{
	vec3		normal;
	vec3		tangent;
	vec3		bitangent;
	int			returnCode;
	uint32_t	index;

	glm_vec3_copy(((vec3 *)vk->core.buffer.vector3.bufferCPU)[vertex.normalIndex], normal);	
	glm_vec3_copy(((vec3 *)vk->core.buffer.vector3.bufferCPU)[vertex.tangentIndex], tangent);	

	glm_vec3_crossn(normal, tangent, bitangent);

	if (!objectInfo->bitangent.freeCount)
		return (0);

	returnCode	= kdo_addDataCPU(vk->core.buffer.vector3.bufferCPU, sizeof(vec3), &bitangent, &index);
	if (returnCode == -1)
		kdo_cleanup(vk, ERRLOC, 12);
	if (returnCode == 1)
		objectInfo->bitangent.freeCount--;

	return (index);
}


Kdo_VkObjectInfo	*kdo_initObject(Kdo_Vulkan *vk, uint32_t vertexCount, uint32_t posCount, uint32_t tangentCount , uint32_t bitangentCount, uint32_t normalCount, uint32_t uvCount, uint32_t materialCount, uint32_t textureCount)
{
	Kdo_VkObjectInfo	*objectInfo;

	if (MAX_TEXTURE < vk->core.buffer.texture.countFill + textureCount)
		return (NULL);
	
	if (!(objectInfo	= malloc(sizeof(Kdo_VkObjectInfo))))
		return (NULL);
	if (!(objectInfo->vertex	= malloc(vertexCount * sizeof(Kdo_VkVertex)))
	 || kdo_allocBufferCPU(&vk->core.buffer.vector3, (posCount + tangentCount + bitangentCount + normalCount) * sizeof(vec3))
	 || kdo_allocBufferCPU(&vk->core.buffer.vector2, uvCount * sizeof(vec2))
	 || kdo_allocBufferCPU(&vk->core.buffer.material, materialCount * sizeof(Kdo_VkMaterial))
	 || kdo_allocBufferCPU(&vk->core.buffer.material, materialCount * sizeof(Kdo_VkMaterial))
	 || kdo_allocImageCPU(&vk->core.buffer.texture, textureCount)
	 || (!(objectInfo->pos.matchArray		= calloc(posCount + 1, sizeof(uint32_t))))
	 || (!(objectInfo->tangent.matchArray	= calloc(tangentCount + 1, sizeof(uint32_t))))
	 || (!(objectInfo->bitangent.matchArray	= calloc(bitangentCount + 1, sizeof(uint32_t))))
	 || (!(objectInfo->normal.matchArray	= calloc(normalCount + 1, sizeof(uint32_t))))
	 || (!(objectInfo->uv.matchArray		= calloc(uvCount + 1, sizeof(uint32_t))))
	 || (!(objectInfo->material.matchArray	= calloc(materialCount + 1, sizeof(uint32_t))))
	 || (!(objectInfo->texture.matchArray	= calloc(textureCount + 1, sizeof(uint32_t)))))
		return (NULL);
	glm_mat4_identity(objectInfo->object.modelMat);
	glm_mat4_identity(objectInfo->object.normalMat);
	objectInfo->vertexCount							= 0;
	objectInfo->pos.freeCount						= posCount;
	objectInfo->tangent.freeCount					= tangentCount;
	objectInfo->bitangent.freeCount					= bitangentCount;
	objectInfo->normal.freeCount					= normalCount;
	objectInfo->uv.freeCount						= uvCount;
	objectInfo->material.freeCount					= materialCount;
	objectInfo->texture.freeCount					= textureCount;
	objectInfo->object.drawCommand.vertexCount		= 0;
	objectInfo->object.drawCommand.instanceCount	= 0;
	objectInfo->object.drawCommand.firstVertex		= 0;
	objectInfo->object.drawCommand.firstInstance	= 0;

	return (objectInfo);
}

int	kdo_addPosObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec3 pos)
{
	int			returnCode;
	uint32_t	matchIndex;

	if (!objectInfo->pos.freeCount || !index)
		return (1);

	returnCode	= kdo_addDataCPU(&vk->core.buffer.vector3, sizeof(vec3), &pos, &index);
	if (returnCode == -1)
		kdo_cleanup(vk, ERRLOC, 12);
	if (returnCode == 1)
		objectInfo->pos.freeCount--;
	
	objectInfo->pos.matchArray[index] = matchIndex;

	return (0);
}

int	kdo_addTangentObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec3 tangent)
{
	int			returnCode;
	uint32_t	matchIndex;

	if (!objectInfo->tangent.freeCount || !index)
		return (1);

	returnCode	= kdo_addDataCPU(&vk->core.buffer.vector3, sizeof(vec3), &tangent, &index);
	if (returnCode == -1)
		kdo_cleanup(vk, ERRLOC, 12);
	if (returnCode == 1)
		objectInfo->tangent.freeCount--;
	
	objectInfo->tangent.matchArray[index] = matchIndex;

	return (0);
}

int	kdo_addBitangentObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec3 bitangent)
{
	int			returnCode;
	uint32_t	matchIndex;

	if (!objectInfo->bitangent.freeCount || !index)
		return (1);

	returnCode	= kdo_addDataCPU(&vk->core.buffer.vector3, sizeof(vec3), &bitangent, &index);
	if (returnCode == -1)
		kdo_cleanup(vk, ERRLOC, 12);
	if (returnCode == 1)
		objectInfo->bitangent.freeCount--;
	
	objectInfo->bitangent.matchArray[index] = matchIndex;

	return (0);
}

int	kdo_addNormalObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec3 normal)
{
	int			returnCode;
	uint32_t	matchIndex;

	if (!objectInfo->normal.freeCount || !index)
		return (1);

	returnCode	= kdo_addDataCPU(&vk->core.buffer.vector3, sizeof(vec3), &normal, &index);
	if (returnCode == -1)
		kdo_cleanup(vk, ERRLOC, 12);
	if (returnCode == 1)
		objectInfo->normal.freeCount--;
	
	objectInfo->normal.matchArray[index] = matchIndex;

	return (0);
}

int	kdo_addUvObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec2 uv)
{
	int			returnCode;
	uint32_t	matchIndex;

	if (!objectInfo->uv.freeCount || !index)
		return (1);

	returnCode	= kdo_addDataCPU(&vk->core.buffer.vector2, sizeof(vec2), &uv, &index);
	if (returnCode == -1)
		kdo_cleanup(vk, ERRLOC, 12);
	if (returnCode == 1)
		objectInfo->uv.freeCount--;
	
	objectInfo->uv.matchArray[index] = matchIndex;

	return (0);
}

int	kdo_addTextureObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, char *path)
{
	int			returnCode;
	uint32_t	matchIndex;

	if (!objectInfo->texture.freeCount || !index)
		return (1);

	returnCode	= kdo_addImageCPU(&vk->core.buffer.texture, path, &index);
	if (returnCode == -1)
		kdo_cleanup(vk, ERRLOC, 12);
	if (returnCode == 1)
		objectInfo->texture.freeCount--;
	
	objectInfo->texture.matchArray[index] = matchIndex;

	return (0);
}

int	kdo_addMaterialObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, Kdo_VkMaterial material)
{
	int			returnCode;
	uint32_t	matchIndex;

	if (!objectInfo->material.freeCount || !index)
		return (1);

	material.ambientMap				= objectInfo->texture.matchArray[material.ambientMap];
	material.diffuseMap				= objectInfo->texture.matchArray[material.diffuseMap];
	material.specularMap			= objectInfo->texture.matchArray[material.specularMap];
	material.emissiveMap			= objectInfo->texture.matchArray[material.emissiveMap];
	material.transmittanceMap		= objectInfo->texture.matchArray[material.transmittanceMap];
	material.transmissionFilterMap	= objectInfo->texture.matchArray[material.transmissionFilterMap];
	material.shininessMap			= objectInfo->texture.matchArray[material.shininessMap];
	material.refractionMap			= objectInfo->texture.matchArray[material.refractionMap];
	material.disolveMap				= objectInfo->texture.matchArray[material.disolveMap];
	material.bumpMap				= objectInfo->texture.matchArray[material.bumpMap];

	returnCode	= kdo_addDataCPU(&vk->core.buffer.material, sizeof(Kdo_VkMaterial), &material, &index);
	if (returnCode == -1)
		kdo_cleanup(vk, ERRLOC, 12);
	if (returnCode == 1)
		objectInfo->material.freeCount--;
	
	objectInfo->material.matchArray[index] = matchIndex;

	return (0);
}

int	kdo_addTriangleObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, Kdo_VkVertex vertex[3])
{
	int			validUv;
	vec3		rawTangent;

	if (!vertex[0].posIndex || !vertex[1].posIndex || !vertex[2].posIndex)
		return (1);

	objectInfo->vertex[objectInfo->vertexCount + 0].posIndex			= objectInfo->pos.matchArray[vertex[0].posIndex];
	objectInfo->vertex[objectInfo->vertexCount + 1].posIndex			= objectInfo->pos.matchArray[vertex[1].posIndex];
	objectInfo->vertex[objectInfo->vertexCount + 2].posIndex			= objectInfo->pos.matchArray[vertex[2].posIndex];


	validUv = vertex[0].uvIndex && vertex[1].uvIndex && vertex[2].uvIndex;

	objectInfo->vertex[objectInfo->vertexCount + 0].uvIndex				= objectInfo->uv.matchArray[vertex[0].uvIndex];
	objectInfo->vertex[objectInfo->vertexCount + 1].uvIndex				= objectInfo->uv.matchArray[vertex[1].uvIndex];
	objectInfo->vertex[objectInfo->vertexCount + 2].uvIndex				= objectInfo->uv.matchArray[vertex[2].uvIndex];


	if (!vertex[0].normalIndex || !vertex[1].normalIndex || !vertex[2].normalIndex)
		objectInfo->normal.matchArray[0] = kdo_calculateNormal(vk, objectInfo, vertex);

	objectInfo->vertex[objectInfo->vertexCount + 0].normalIndex			= objectInfo->normal.matchArray[vertex[0].normalIndex];
	objectInfo->vertex[objectInfo->vertexCount + 0].normalIndex			= objectInfo->normal.matchArray[vertex[0].normalIndex];
	objectInfo->vertex[objectInfo->vertexCount + 0].normalIndex			= objectInfo->normal.matchArray[vertex[0].normalIndex];


	if (validUv && (!vertex[0].tangentIndex || !vertex[1].tangentIndex || !vertex[2].tangentIndex))
		kdo_calculateRawTangent(vk, vertex, rawTangent);

	if (validUv && !vertex[0].tangentIndex)
		objectInfo->vertex[objectInfo->vertexCount + 0].tangentIndex   = kdo_calculateTangent(vk, objectInfo, vertex[0], rawTangent);
	else
		objectInfo->vertex[objectInfo->vertexCount + 0].tangentIndex   = objectInfo->tangent.matchArray[vertex[0].tangentIndex];
	if (validUv && !vertex[1].tangentIndex)
		objectInfo->vertex[objectInfo->vertexCount + 1].tangentIndex   = kdo_calculateTangent(vk, objectInfo, vertex[1], rawTangent);
	else
		objectInfo->vertex[objectInfo->vertexCount + 1].tangentIndex   = objectInfo->tangent.matchArray[vertex[1].tangentIndex];
	if (validUv && !vertex[2].tangentIndex)
		objectInfo->vertex[objectInfo->vertexCount + 2].tangentIndex   = kdo_calculateTangent(vk, objectInfo, vertex[2], rawTangent);
	else
		objectInfo->vertex[objectInfo->vertexCount + 2].tangentIndex   = objectInfo->tangent.matchArray[vertex[2].tangentIndex];


	if (!vertex[0].bitangentIndex)
		 objectInfo->vertex[objectInfo->vertexCount + 0].bitangentIndex	= kdo_calculateBitangent(vk, objectInfo, vertex[0]);
	else
		objectInfo->vertex[objectInfo->vertexCount + 0].bitangentIndex	=  objectInfo->bitangent.matchArray[vertex[0].bitangentIndex];
	if (!vertex[1].bitangentIndex)
		 objectInfo->vertex[objectInfo->vertexCount + 1].bitangentIndex	= kdo_calculateBitangent(vk, objectInfo, vertex[1]);
	else
		objectInfo->vertex[objectInfo->vertexCount + 1].bitangentIndex	=  objectInfo->bitangent.matchArray[vertex[1].bitangentIndex];
	if (!vertex[2].bitangentIndex)
		 objectInfo->vertex[objectInfo->vertexCount + 2].bitangentIndex	= kdo_calculateBitangent(vk, objectInfo, vertex[2]);
	else
		objectInfo->vertex[objectInfo->vertexCount + 2].bitangentIndex	=  objectInfo->bitangent.matchArray[vertex[2].bitangentIndex];


	objectInfo->vertex[objectInfo->vertexCount + 0].mtlIndex			= objectInfo->material.matchArray[vertex[0].mtlIndex];
	objectInfo->vertex[objectInfo->vertexCount + 1].mtlIndex			= objectInfo->material.matchArray[vertex[1].mtlIndex];
	objectInfo->vertex[objectInfo->vertexCount + 2].mtlIndex			= objectInfo->material.matchArray[vertex[2].mtlIndex];

	objectInfo->vertexCount += 3;

	return (0);
}

void	kdo_updateBufferGPU(Kdo_Vulkan *vk, Kdo_VkBufferCoreElement *buffer)
{
	if (buffer->bufferGPU.sizeUsed + buffer->bufferGPU.sizeFree < buffer->sizeFill)	

}

void	kdo_loadObject(Kdo_Vulkan *vk, uint32_t objectCount, Kdo_VkObjectInfo *info)
{

}
