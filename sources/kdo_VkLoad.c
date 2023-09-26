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

static uint32_t	kdo_loadMesh(Kdo_Vulkan *vk, Kdo_Vertex *vertexIn, uint32_t vertexInCount)
{
	uint32_t	*indexOut;
	uint32_t	vertexOutCount;
	Kdo_Vertex	*vertexOut;

	vertexOutCount = kdo_splitMesh(vk, vertexIn, vertexInCount, &vertexOut, &indexOut);
	kdo_setData(vk, &vk->core.buffer.vertex, vertexOut, vertexOutCount * sizeof(Kdo_Vertex), vk->core.buffer.vertex.sizeUsed);
	kdo_setData(vk, &vk->core.buffer.index, indexOut, vertexInCount * sizeof(uint32_t), vk->core.buffer.index.sizeUsed);

	free(vertexOut);
	free(indexOut);

	return (vertexOutCount);
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

static int	kdo_allocBufferCpy(Kdo_VkBufferCoreElement *buffer, size_t needSize)
{
	if (buffer->sizeFree < needSize)
		if (!(buffer->bufferCpy	= realloc(buffer->bufferCpy, buffer->sizeUsed + needSize)))
			return (1);
	buffer->sizeUsed	+= needSize;
	buffer->sizeFree	-= glm_min(buffer->sizeFree, needSize);

	return (0);
}

static void	kdo_freeBufferCpy(Kdo_VkBufferCoreElement *buffer, size_t freeSize)
{
	buffer->sizeFree	+= glm_min(buffer->sizeUsed, freeSize);	
	buffer->sizeUsed	-= glm_min(buffer->sizeUsed, freeSize);
}

static int	kdo_allocImageCpy(Kdo_VkImageCoreElement *image, uint32_t needCount)
{
	if (image->countFree < needCount)
		if (!(image->name	= realloc(image->name, (image->countUsed + needCount) * sizeof(char *))))
			return (1);
	image->countUsed	+= needCount;
	image->countFree	-= glm_min(image->countFree, needCount);

	return (0);
}

static void	kdo_freeImageCpy(Kdo_VkImageCoreElement *image, uint32_t freeCount)
{
	image->countFree	+= glm_min(image->countUsed, freeCount);	
	image->countUsed	-= glm_min(image->countUsed, freeCount);
}



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

void kdo_findNormal(vec3 pos1, vec3 pos2, vec3 pos3, vec3 normal)
{
  vec3    x;
  vec3    y;

  glm_vec3_sub(pos2, pos3, x);
  glm_vec3_sub(pos2, pos1, y);
  glm_vec3_crossn(x, y, normal);
}

void kdo_findRawTangent(vec3 pos1, vec3 pos2, vec3 pos3, vec2 uv1, vec2 uv2, vec2 uv3, vec3 rawTangent)
{
  vec3    deltaPos1;
  vec3    deltaPos2;
  vec2    deltaUV1;
  vec2    deltaUV2;
  float   factor;

  glm_vec3_sub(pos2, pos1, deltaPos1);
  glm_vec3_sub(pos3, pos1, deltaPos2);
  glm_vec2_sub(uv2, uv1, deltaUV1);
  glm_vec2_sub(uv3, uv1, deltaUV2);

  factor = deltaUV1[0] * deltaUV2[1] - deltaUV1[1] * deltaUV2[0];
  if (factor == 0.0f)
  {
	  glm_vec3_zero(rawTangent);
	  return ;
  }

  glm_vec3_scale(deltaPos1, deltaUV2[1], deltaPos1);
  glm_vec3_scale(deltaPos2, deltaUV1[1], deltaPos2);
  glm_vec3_sub(deltaPos1, deltaPos2, rawTangent);
  glm_vec3_divs(rawTangent, factor, rawTangent);
}

void kdo_findTangent(vec3 rawTangent, vec2 normal, vec3 tangent)
{
  vec3            adjustVector;

  glm_vec3_proj(rawTangent, normal, adjustVector);
  glm_vec3_sub(rawTangent, adjustVector, tangent);
  glm_vec3_normalize(tangent);
}



Kdo_VkObjectInfo	*kdo_createObject(Kdo_Vulkan *vk, uint32_t vertexCount, uint32_t posCount, uint32_t tangentCount , uint32_t bitangentCount, uint32_t normalCount, uint32_t uvCount, uint32_t materialCount, uint32_t textureCount)
{
	Kdo_VkObjectInfo	*objectInfo;

	
	if (!(objectInfo	= malloc(sizeof(Kdo_VkObjectInfo))))
		return (NULL);

	if (kdo_allocBufferCpy(&vk->core.buffer.vertex, vertexCount * sizeof(Kdo_VkVertex))
	 || kdo_allocBufferCpy(&vk->core.buffer.vector3, (posCount + tangentCount + bitangentCount + normalCount) * sizeof(vec3))
	 || kdo_allocBufferCpy(&vk->core.buffer.vector2, uvCount * sizeof(vec2))
	 || kdo_allocBufferCpy(&vk->core.buffer.material, materialCount * sizeof(Kdo_VkMaterial))
	 || kdo_allocBufferCpy(&vk->core.buffer.material, materialCount * sizeof(Kdo_VkMaterial))
	 || kdo_allocImageCpy(&vk->core.buffer.texture, textureCount)
	 || (!(objectInfo->posMatchArray		= calloc(posCount + 1, sizeof(uint32_t))))
	 || (!(objectInfo->tangentMatchArray	= calloc(tangentCount + 1, sizeof(uint32_t))))
	 || (!(objectInfo->bitangentMatchArray	= calloc(bitangentCount + 1, sizeof(uint32_t))))
	 || (!(objectInfo->normalMatchArray		= calloc(normalCount + 1, sizeof(uint32_t))))
	 || (!(objectInfo->uvMatchArray			= calloc(uvCount + 1, sizeof(uint32_t))))
	 || (!(objectInfo->materialMatchArray	= calloc(materialCount + 1, sizeof(uint32_t))))
	 || (!(objectInfo->textureMatchArray	= calloc(textureCount + 1, sizeof(uint32_t)))))
		return (NULL);

	glm_mat4_identity(objectInfo->object.modelMat);
	glm_mat4_identity(objectInfo->object.normalMat);
	objectInfo->freeVertexCount						= vertexCount;
	objectInfo->freePosCount						= posCount;
	objectInfo->freeTangentCount					= tangentCount;
	objectInfo->freeBitangentCount					= bitangentCount;
	objectInfo->freeNormalCount						= normalCount;
	objectInfo->freeUvCount							= uvCount;
	objectInfo->freeMaterialCount					= materialCount;
	objectInfo->freeTextureCount					= textureCount;
	objectInfo->object.drawCommand.vertexCount		= 0;
	objectInfo->object.drawCommand.instanceCount	= 1;
	objectInfo->object.drawCommand.firstVertex		= vk->core.buffer.vertex.countUpdate + vk->core.buffer.vertex.countNoUpdate;
	objectInfo->object.drawCommand.firstInstance	= 0;

	return (objectInfo);
}

int	kdo_addPos(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec3 pos)
{
	uint32_t matchIndex;

	if (!objectInfo->freePosCount || !index)
		return (1);

	matchIndex = vk->core.buffer.vector3.countUpdate + vk->core.buffer.vector3.countNoUpdate;
	if (!(kdo_BSTaddData(&vk->core.buffer.vector3.BSTRoot, vk->core.buffer.vector3.bufferCpy, sizeof(vec3), &matchIndex, &pos)))
		return (1);
	if (matchIndex == vk->core.buffer.vector3.countUpdate + vk->core.buffer.vector3.countNoUpdate)
		objectInfo->freePosCount--;
	
	objectInfo->posMatchArray[index] = matchIndex;

	return (0);
}

int	kdo_addTangent(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec3 tangent)
{
	uint32_t matchIndex;

	if (!objectInfo->freeTangentCount || !index)
		return (1);

	matchIndex = vk->core.buffer.vector3.countUpdate + vk->core.buffer.vector3.countNoUpdate;
	if (!(kdo_BSTaddData(&vk->core.buffer.vector3.BSTRoot, vk->core.buffer.vector3.bufferCpy, sizeof(vec3), &matchIndex, &tangent)))
		return (1);
	if (matchIndex == vk->core.buffer.vector3.countUpdate + vk->core.buffer.vector3.countNoUpdate)
		objectInfo->freeTangentCount--;
	
	objectInfo->tangentMatchArray[index] = matchIndex;

	return (0);
}

int	kdo_addBitangent(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec3 bitangent)
{
	uint32_t matchIndex;

	if (!objectInfo->freeBitangentCount || !index)
		return (1);

	matchIndex = vk->core.buffer.vector3.countUpdate + vk->core.buffer.vector3.countNoUpdate;
	if (!(kdo_BSTaddData(&vk->core.buffer.vector3.BSTRoot, vk->core.buffer.vector3.bufferCpy, sizeof(vec3), &matchIndex, bitangent)))
		return (1);
	if (matchIndex == vk->core.buffer.vector3.countUpdate + vk->core.buffer.vector3.countNoUpdate)
		objectInfo->freeBitangentCount--;
	
	objectInfo->bitangentMatchArray[index] = matchIndex;

	return (0);
}

int	kdo_addNormal(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec3 normal)
{
	uint32_t matchIndex;

	if (!objectInfo->freeNormalCount || !index)
		return (1);

	matchIndex = vk->core.buffer.vector3.countUpdate + vk->core.buffer.vector3.countNoUpdate;
	if (!(kdo_BSTaddData(&vk->core.buffer.vector3.BSTRoot, vk->core.buffer.vector3.bufferCpy, sizeof(vec3), &matchIndex, &normal)))
		return (1);
	if (matchIndex == vk->core.buffer.vector3.countUpdate + vk->core.buffer.vector3.countNoUpdate)
		objectInfo->freeNormalCount--;
	
	objectInfo->normalMatchArray[index] = matchIndex;

	return (0);
}

int	kdo_addUv(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, vec2 uv)
{
	uint32_t matchIndex;

	if (!objectInfo->freeUvCount || !index)
		return (1);

	matchIndex = vk->core.buffer.vector2.countUpdate + vk->core.buffer.vector2.countNoUpdate;
	if (!(kdo_BSTaddData(&vk->core.buffer.vector2.BSTRoot, vk->core.buffer.vector2.bufferCpy, sizeof(vec2), &matchIndex, &uv)))
		return (1);
	if (matchIndex == vk->core.buffer.vector2.countUpdate + vk->core.buffer.vector2.countNoUpdate)
		objectInfo->freeUvCount--;
	
	objectInfo->uvMatchArray[index] = matchIndex;

	return (0);
}

int	kdo_addMtl(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, Kdo_VkMaterial mtl)
{
	uint32_t matchIndex;

	if (!objectInfo->freeMaterialCount || !index)
		return (1);

	matchIndex = vk->core.buffer.material.countUpdate + vk->core.buffer.material.countNoUpdate;
	if (!(kdo_BSTaddData(&vk->core.buffer.material.BSTRoot, vk->core.buffer.material.bufferCpy, sizeof(Kdo_VkMaterial), &matchIndex, &mtl)))
		return (1);
	if (matchIndex == vk->core.buffer.material.countUpdate + vk->core.buffer.material.countNoUpdate)
		objectInfo->freeMaterialCount--;
	
	objectInfo->materialMatchArray[index] = matchIndex;

	return (0);
}

int	kdo_addTexture(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, uint32_t index, char *path)
{
	uint32_t matchIndex;

	if (!objectInfo->freeTextureCount || !index)
		return (1);

	matchIndex = vk->core.buffer.texture.countUpdate + vk->core.buffer.texture.countNoUpdate;
	if (!(kdo_BSTaddStr(&vk->core.buffer.texture.BSTRoot, vk->core.buffer.texture.name, &matchIndex, basename(path))))
		return (1);
	if (matchIndex == vk->core.buffer.texture.countUpdate + vk->core.buffer.texture.countNoUpdate)
		objectInfo->freeMaterialCount--;
	
	objectInfo->textureMatchArray[index] = matchIndex;

	return (0);
}

int	kdo_addTriangle(Kdo_Vulkan *vk, Kdo_VkObjectInfo *objectInfo, Kdo_VkVertex vertex[3])
{

	return (0);
}

void	kdo_loadObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo info)
{
}
