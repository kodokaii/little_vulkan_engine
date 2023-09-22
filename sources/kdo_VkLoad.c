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
	VkDescriptorImageInfo			writeImageInfo[MAX_TEXTURES];
	VkWriteDescriptorSet			descriptorWrite;

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

Kdo_ShMaterial	kdo_defaultMaterial(void)
{
	Kdo_ShMaterial	defaultMtl	=	{{0, 0, 0}, 0,	//ambient
									{1, 1, 1}, 0,	//diffuse
									{1, 1, 1}, 0,	//specular
									{1, 1, 1}, 0,	//emissive
									{1, 1, 1}, 0,	//transmittance
									{1, 1, 1}, 0,	//transmissionFilter
									400, 0,			//shininess
									1, 0,			//refractionIndex
									1, 0,			//disolve
									0,				//illum
									0};				//bumpMap
	return (defaultMtl);
}

void	kdo_loadObject(Kdo_Vulkan *vk, Kdo_VkObjectInfo info)
{
	Kdo_ShObjectMap	objectMap;
	uint32_t		*materialMap;

	if (!(materialMap = malloc(info.materialCount * sizeof(uint32_t))))
		kdo_cleanup(vk, ERRLOC, 12);

	glm_mat4_identity(objectMap.modelMat);
	glm_mat4_identity(objectMap.normalMat);
	objectMap.materialOffset				= vk->core.count.materialMap;
	objectMap.drawCommand.indexCount		= info.vertexCount;
	objectMap.drawCommand.instanceCount		= 1;
	objectMap.drawCommand.firstIndex		= vk->core.count.index;
	objectMap.drawCommand.vertexOffset		= vk->core.count.vertex;
	objectMap.drawCommand.firstInstance		= vk->core.count.object;
	kdo_setData(vk, &vk->core.buffer.object, &objectMap, sizeof(Kdo_ShObjectMap), vk->core.buffer.object.sizeUsed);

	for (uint32_t currentMaterial = 0; currentMaterial < info.materialCount; currentMaterial++)
	{
		if (kdo_loadTexture(vk, info.texturePath[info.material[currentMaterial].map_Kd]))
			info.material[currentMaterial].map_Kd	= 0;
		else
			info.material[currentMaterial].map_Kd	= vk->core.buffer.textures.imageCount - 1;

		if (kdo_loadTexture(vk, info.texturePath[info.material[currentMaterial].map_Ks]))
			info.material[currentMaterial].map_Ks	= 0;
		else
			info.material[currentMaterial].map_Ks	= vk->core.buffer.textures.imageCount - 1;

		if (kdo_loadTexture(vk, info.texturePath[info.material[currentMaterial].map_Bump]))
			info.material[currentMaterial].map_Bump	= 0;
		else
			info.material[currentMaterial].map_Bump	= vk->core.buffer.textures.imageCount - 1;

		kdo_setData(vk, &vk->core.buffer.materials, &info.material[currentMaterial], sizeof(Kdo_ShMaterial), vk->core.buffer.materials.sizeUsed);

		materialMap[currentMaterial] = vk->core.count.materialMap + currentMaterial;
	}
	kdo_setData(vk, &vk->core.buffer.materialMap, materialMap, info.materialCount * sizeof(uint32_t), vk->core.buffer.materialMap.sizeUsed);

	vk->core.count.vertex		+= kdo_loadMesh(vk, info.vertex, info.vertexCount);
	vk->core.count.index		+= info.vertexCount;
	vk->core.count.materialMap	+= info.materialCount;
	vk->core.count.materials	+= info.materialCount;
	vk->core.count.object		+= 1;

	for (uint32_t i = 0; i < info.textureCount; i++)
		free(info.texturePath[i]);

	free(info.vertex);
	free(info.material);
	free(info.texturePath);
	free(materialMap);
}
