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

#include "kdo_VkCore.h"

#define FAST_OBJ_IMPLEMENTATION
#include "objLoader/fast_obj.h"

static float	kdo_vec2Cmp(const vec2 vecteur1, const vec2 vecteur2)
{
	if (vecteur1[0] != vecteur2[0])
		return (vecteur1[0] - vecteur2[0]);
	return (vecteur1[1] - vecteur2[1]);
}

static float	kdo_vec3Cmp(const vec3 vecteur1, const vec3 vecteur2)
{
	if (vecteur1[0] != vecteur2[0])
		return (vecteur1[0] - vecteur2[0]);
	if (vecteur1[1] != vecteur2[1])
		return (vecteur1[1] - vecteur2[1]);
	return (vecteur1[2] - vecteur2[2]);
}

static float	kdo_vertexCmp(const Kdo_Vertex vertex1, const Kdo_Vertex vertex2)
{
	float	res;
	
	if ((res = kdo_vec3Cmp(vertex1.pos, vertex2.pos)))
		return (res);
	if ((res = kdo_vec3Cmp(vertex1.normal, vertex2.normal)))
		return (res);
	return (kdo_vec2Cmp(vertex1.tex, vertex2.tex));
}

static uint32_t	kdo_sortMesh(Kdo_Vertex *vertex, uint32_t toSortCount, uint32_t *sortIndex, uint32_t *index)
{
	uint32_t	beforeCount	= 0;
	uint32_t	sameCount	= 1;
	uint32_t	afterCount	= 0;
	uint32_t	buffer;
	float		res;

	while (beforeCount + sameCount + afterCount < toSortCount)
	{
		res = kdo_vertexCmp(vertex[sortIndex[beforeCount]], vertex[sortIndex[beforeCount + sameCount]]);

		if (res < 0)
		{
			buffer											= sortIndex[beforeCount];
			sortIndex[beforeCount]							= sortIndex[beforeCount + sameCount];
			sortIndex[beforeCount + sameCount]				= buffer;

			index[sortIndex[beforeCount]]					-= sameCount;
			index[sortIndex[beforeCount + sameCount]]		+= sameCount;

			beforeCount++;
		}
		else if (0 < res)
		{
			buffer											= sortIndex[beforeCount + sameCount];
			sortIndex[beforeCount + sameCount]				= sortIndex[toSortCount - afterCount - 1];
			sortIndex[toSortCount - afterCount - 1]			= buffer;

			index[sortIndex[beforeCount + sameCount]]		-= toSortCount - afterCount - beforeCount - sameCount - 1;
			index[sortIndex[toSortCount - afterCount - 1]]	+= toSortCount - afterCount - beforeCount - sameCount - 1;

			afterCount++;
		}
		else
			sameCount++;
	}

	if (1 < beforeCount)
		sameCount += kdo_sortMesh(vertex, beforeCount, sortIndex, index);
	if (1 < afterCount)
		sameCount += kdo_sortMesh(vertex, afterCount, &sortIndex[toSortCount - afterCount], index);

	return (sameCount - 1);
}


static uint32_t	kdo_splitMesh(Kdo_Vulkan *vk, Kdo_Vertex *vertexIn, uint32_t vertexInCount, Kdo_Vertex **vertexOut, uint32_t **indexOut)
{
	uint32_t	*sortVertexIndex;
	uint32_t	vertexOutCount;
	uint32_t	currentVertexIn;
	uint32_t	currentVertexOut;
	uint32_t	currentSameVertexCount;

	if (!(*indexOut			= malloc(vertexInCount * sizeof(uint32_t))))
			kdo_cleanup(vk, ERRLOC, 12);
	if (!(sortVertexIndex	= malloc(vertexInCount * sizeof(uint32_t))))
			kdo_cleanup(vk, ERRLOC, 12);

	for (uint32_t i = 0; i < vertexInCount; i++)
	{
		(*indexOut)[i]		= i;
		sortVertexIndex[i]	= i;
	}
	vertexOutCount = vertexInCount - kdo_sortMesh(vertexIn, vertexInCount, sortVertexIndex, *indexOut);
	if (!(*vertexOut = malloc(vertexOutCount * sizeof(Kdo_Vertex))))
		kdo_cleanup(vk, ERRLOC, 12);

	currentVertexIn = 1;
	currentVertexOut = 1;
	currentSameVertexCount = 0;
	(*vertexOut)[0] = vertexIn[sortVertexIndex[0]];
	while (currentVertexIn < vertexInCount)
	{
		if (kdo_vertexCmp(vertexIn[sortVertexIndex[currentVertexIn]], vertexIn[sortVertexIndex[currentVertexIn - 1]]))
			(*vertexOut)[currentVertexOut++] = vertexIn[sortVertexIndex[currentVertexIn]];
		else
			currentSameVertexCount++;
		(*indexOut)[sortVertexIndex[currentVertexIn]] -= currentSameVertexCount;
		currentVertexIn++;
	}

	free(sortVertexIndex);
	return (vertexOutCount);
}

static void kdo_findNormal(vec3 vecteur1, vec3 vecteur2, vec3 vecteur3, vec3 *normal)
{
	vec3	x;
	vec3	y;

	glm_vec3_sub(vecteur2, vecteur3, x);
	glm_vec3_sub(vecteur2, vecteur1, y);
	glm_vec3_crossn(x, y, *normal);
}

static void	kdo_initSampler(Kdo_Vulkan *vk) {
	VkSamplerCreateInfo		samplerInfo;

	samplerInfo.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext					= NULL;
	samplerInfo.flags					= 0;
	samplerInfo.magFilter				= VK_FILTER_LINEAR;
	samplerInfo.minFilter				= VK_FILTER_LINEAR;
	samplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.mipLodBias				= 0.0f;
	samplerInfo.anisotropyEnable		= VK_TRUE;
	samplerInfo.maxAnisotropy			= 16.0f;
	samplerInfo.compareEnable			= VK_FALSE;
	samplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
	samplerInfo.minLod					= 0.0f;
	samplerInfo.maxLod					= 0.0f;
	samplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates	= VK_FALSE;
	if (vkCreateSampler(vk->device.path, &samplerInfo, NULL, &vk->core.sampler.basic) != VK_SUCCESS)
		kdo_cleanup(vk, "Sampler creation failed", 30);
}

static void kdo_initDescriptorPool(Kdo_Vulkan *vk)
{
	VkDescriptorPoolSize			descriptorPoolSize[6];
	VkDescriptorPoolCreateInfo		descriptorPoolInfo;
	VkDescriptorSetAllocateInfo     allocInfo;
	VkDescriptorImageInfo			imageInfo;
	VkWriteDescriptorSet			descriptorWrite;

	descriptorPoolSize[0].type				= VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorPoolSize[0].descriptorCount	= 1;

	descriptorPoolSize[1].type				= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorPoolSize[1].descriptorCount	= 128;

	descriptorPoolSize[2].type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize[2].descriptorCount	= 1;

	descriptorPoolSize[3].type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize[3].descriptorCount	= 1;

	descriptorPoolSize[4].type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize[4].descriptorCount	= 1;

	descriptorPoolSize[5].type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize[5].descriptorCount	= 1;

	descriptorPoolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext			= NULL;
	descriptorPoolInfo.flags			= 0;
	descriptorPoolInfo.maxSets			= 1;
	descriptorPoolInfo.poolSizeCount	= 6;
	descriptorPoolInfo.pPoolSizes		= descriptorPoolSize;
	if (vkCreateDescriptorPool(vk->device.path, &descriptorPoolInfo, NULL, &vk->core.descriptorPool) != VK_SUCCESS)
		kdo_cleanup(vk, "Descriptor pool creation failed", 31);

	allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext                 = NULL;
	allocInfo.descriptorPool        = vk->core.descriptorPool;
	allocInfo.descriptorSetCount    = 1;
	allocInfo.pSetLayouts           = &vk->graphicsPipeline.descriptorLayout;
	if (vkAllocateDescriptorSets(vk->device.path, &allocInfo, &vk->core.descriptorSet) != VK_SUCCESS)
              kdo_cleanup(vk, "Descriptor set allocation failed", 32);

	imageInfo.sampler		= vk->core.sampler.basic;
	imageInfo.imageView		= 0;
	imageInfo.imageLayout	= 0;

	descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext				= NULL;
	descriptorWrite.dstSet				= vk->core.descriptorSet;
	descriptorWrite.dstBinding			= 2;
	descriptorWrite.dstArrayElement		= 0;
	descriptorWrite.descriptorCount		= 1;
	descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorWrite.pImageInfo			= &imageInfo;
	descriptorWrite.pBufferInfo			= NULL;
	descriptorWrite.pTexelBufferView	= NULL;
	vkUpdateDescriptorSets(vk->device.path, 1, &descriptorWrite, 0, NULL);
}


static void kdo_initDescriptorSets(Kdo_Vulkan *vk)
{	
	VkWriteDescriptorSet			descriptorWrite[4];
	VkDescriptorBufferInfo			writeBufferInfo[4];
	VkDescriptorImageInfo			writeImageInfo[MAX_TEXTURES];

	if (kdo_loadTexture(vk, DEFAULT_TEXTURE))
		kdo_cleanup(vk, "Defautl textures load failed", 34);

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


	for (uint32_t i = 0; i < MAX_TEXTURES; i++)
	{
		writeImageInfo[i].sampler		= NULL;
		writeImageInfo[i].imageView		= vk->core.buffer.textures.image[0].view;
		writeImageInfo[i].imageLayout	= vk->core.buffer.textures.properties.layout;
	}

	descriptorWrite[0].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[0].pNext				= NULL;
	descriptorWrite[0].dstSet				= vk->core.descriptorSet;
	descriptorWrite[0].dstBinding			= 3;
	descriptorWrite[0].dstArrayElement		= 0;
	descriptorWrite[0].descriptorCount		= MAX_TEXTURES;
	descriptorWrite[0].descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorWrite[0].pImageInfo			= writeImageInfo;
	descriptorWrite[0].pBufferInfo			= NULL;
	descriptorWrite[0].pTexelBufferView	= NULL;
	vkUpdateDescriptorSets(vk->device.path, 1, descriptorWrite, 0, NULL);
}

void	kdo_initCore(Kdo_Vulkan *vk)
{
	VkCommandPoolCreateInfo	commandPoolInfo;

	commandPoolInfo.sType               = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext               = NULL;
	commandPoolInfo.flags               = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolInfo.queueFamilyIndex    = vk->device.queues[TRANSFER_QUEUE].familyIndex;
	if (vkCreateCommandPool(vk->device.path, &commandPoolInfo, NULL, &vk->core.transferPool) != VK_SUCCESS)

		kdo_cleanup(vk, "Transfer pool creation failed", 33);
	
	vk->core.buffer.textures		= kdo_newImageBuffer(vk, TEXTURE_BUFFER_SIZE, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, findTextureMemoryFiltrer(vk), WAIT_DEVICE);
	vk->core.buffer.vertex			= kdo_newBuffer(vk, VERTEX_BUFFER_SIZE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, WAIT_DEVICE);
	vk->core.buffer.index			= kdo_newBuffer(vk, INDEX_BUFFER_SIZE, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, WAIT_DEVICE);
	vk->core.buffer.materials		= kdo_newBuffer(vk, MAX_MATERIAL * sizeof(Kdo_ShMaterial), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, WAIT_DEVICE);
	vk->core.buffer.materialMap		= kdo_newBuffer(vk, MAX_MATERIAL_MAP * sizeof(uint32_t), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, WAIT_DEVICE);
	vk->core.buffer.light			= kdo_newBuffer(vk, MAX_LIGHT * sizeof(Kdo_ShLight), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, WAIT_DEVICE);
	vk->core.buffer.object			= kdo_newBuffer(vk, MAX_OBJECT * sizeof(Kdo_ShObjectMap), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, WAIT_DEVICE);

	
	kdo_initSampler(vk);	
	kdo_initDescriptorPool(vk);
	kdo_initDescriptorSets(vk);
}

uint32_t	kdo_loadMesh(Kdo_Vulkan *vk, Kdo_Vertex *vertexIn, uint32_t vertexInCount)
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

uint32_t	kdo_loadTexture(Kdo_Vulkan *vk, char *texturePath)
{
	VkDescriptorImageInfo	writeImageInfo;
	VkWriteDescriptorSet	descriptorWrite;
	Kdo_VkImageFuncInfo		funcInfo = {kdo_imageTextureInfo, kdo_viewTextureInfo};

	if (!texturePath || kdo_appendImage(vk, &vk->core.buffer.textures, texturePath, funcInfo))
		return (1);

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

	return (0);
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
		
		if (kdo_loadTexture(vk, info.texturePath[info.material[currentMaterial].map_Ns]))
			info.material[currentMaterial].map_Ns	= 0;
		else
			info.material[currentMaterial].map_Ns	= vk->core.buffer.textures.imageCount - 1;

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

Kdo_VkObjectInfo	kdo_openObj(Kdo_Vulkan *vk, char *objPath)
{
	fastObjMesh			*mesh;
	Kdo_VkObjectInfo	objectInfo;
	uint32_t			currentFace;
	uint32_t			currentFaceVertex;
	uint32_t			currentVertex;
	uint32_t			vertexOffset;

	objectInfo.vertexCount	= 0;
	mesh					= fast_obj_read(objPath);
	for (currentFace= 0; currentFace < mesh->face_count; currentFace++)
		objectInfo.vertexCount	+= (mesh->face_vertices[currentFace] - 2);
	objectInfo.vertexCount	*= 3;

	objectInfo.materialCount	= mesh->material_count;
	objectInfo.textureCount		= mesh->material_count * 3;

	if (!(objectInfo.vertex			= malloc(objectInfo.vertexCount * sizeof(Kdo_Vertex))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(objectInfo.material		= malloc(objectInfo.materialCount * sizeof(Kdo_ShMaterial))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(objectInfo.texturePath	= malloc(objectInfo.materialCount * 3 * sizeof(char *))))
		kdo_cleanup(vk, ERRLOC, 12);

	for (uint32_t currentMaterial  = 0; currentMaterial  < mesh->material_count; currentMaterial ++)
	{
		objectInfo.material[currentMaterial].Ka[0]		= mesh->materials[currentMaterial].Ka[0];
		objectInfo.material[currentMaterial].Ka[1]		= mesh->materials[currentMaterial].Ka[1];
		objectInfo.material[currentMaterial].Ka[2]		= mesh->materials[currentMaterial].Ka[2];
		objectInfo.material[currentMaterial].Kd[0]		= mesh->materials[currentMaterial].Kd[0];
		objectInfo.material[currentMaterial].Kd[1]		= mesh->materials[currentMaterial].Kd[1];
		objectInfo.material[currentMaterial].Kd[2]		= mesh->materials[currentMaterial].Kd[2];
		objectInfo.material[currentMaterial].Ks[0]		= mesh->materials[currentMaterial].Ks[0];
		objectInfo.material[currentMaterial].Ks[1]		= mesh->materials[currentMaterial].Ks[1];
		objectInfo.material[currentMaterial].Ks[2]		= mesh->materials[currentMaterial].Ks[2];
		objectInfo.material[currentMaterial].Ke[0]		= mesh->materials[currentMaterial].Ke[0];
		objectInfo.material[currentMaterial].Ke[1]		= mesh->materials[currentMaterial].Ke[1];
		objectInfo.material[currentMaterial].Ke[2]		= mesh->materials[currentMaterial].Ke[2];
		objectInfo.material[currentMaterial].Kt[0]		= mesh->materials[currentMaterial].Kt[0];
		objectInfo.material[currentMaterial].Kt[1]		= mesh->materials[currentMaterial].Kt[1];
		objectInfo.material[currentMaterial].Kt[2]		= mesh->materials[currentMaterial].Kt[2];
		objectInfo.material[currentMaterial].Tf[0]		= mesh->materials[currentMaterial].Tf[0];
		objectInfo.material[currentMaterial].Tf[1]		= mesh->materials[currentMaterial].Tf[1];
		objectInfo.material[currentMaterial].Tf[2]		= mesh->materials[currentMaterial].Tf[2];
		objectInfo.material[currentMaterial].Ns			= mesh->materials[currentMaterial].Ns;
		objectInfo.material[currentMaterial].Ni			= mesh->materials[currentMaterial].Ni;
		objectInfo.material[currentMaterial].d			= mesh->materials[currentMaterial].d;
		objectInfo.material[currentMaterial].illum		= mesh->materials[currentMaterial].illum;

		objectInfo.material[currentMaterial].map_Kd		= currentMaterial * 3;
		objectInfo.material[currentMaterial].map_Ns		= currentMaterial * 3 + 1;
		objectInfo.material[currentMaterial].map_Bump	= currentMaterial * 3 + 2;

		if (mesh->materials[currentMaterial].map_Kd.path)
		{
			if (!(objectInfo.texturePath[currentMaterial * 3]		= strdup(mesh->materials[currentMaterial].map_Kd.path)))
				kdo_cleanup(vk, ERRLOC, 12);
		}
		else
			objectInfo.texturePath[currentMaterial * 3]				= NULL;

		if (mesh->materials[currentMaterial].map_Ns.path)
		{
			if (!(objectInfo.texturePath[currentMaterial * 3 + 1]	= strdup(mesh->materials[currentMaterial].map_Ns.path)))
				kdo_cleanup(vk, ERRLOC, 12);
		}
		else
			objectInfo.texturePath[currentMaterial * 3 + 1]			= NULL;

		if (mesh->materials[currentMaterial].map_bump.path)
		{
			if (!(objectInfo.texturePath[currentMaterial * 3 + 2]	= strdup(mesh->materials[currentMaterial].map_bump.path)))
				kdo_cleanup(vk, ERRLOC, 12);
		}
		else
			objectInfo.texturePath[currentMaterial * 3 + 2]			= NULL;
	}

	currentFaceVertex   = 0;
	currentVertex       = 0;
	for(currentFace = 0; currentFace < mesh->face_count; currentFace++)
	{
		for (vertexOffset = 0; 3 <= mesh->face_vertices[currentFace] - vertexOffset; vertexOffset++)
		{
			objectInfo.vertex[currentVertex + vertexOffset * 3].pos[0]					=	mesh->positions[mesh->indices[currentFaceVertex].p * 3];
			objectInfo.vertex[currentVertex + vertexOffset * 3].pos[1]					=	mesh->positions[mesh->indices[currentFaceVertex].p * 3 + 1];
			objectInfo.vertex[currentVertex + vertexOffset * 3].pos[2]					=	mesh->positions[mesh->indices[currentFaceVertex].p * 3 + 2] * -1;
			objectInfo.vertex[currentVertex + vertexOffset * 3].tex[0]					=	mesh->texcoords[mesh->indices[currentFaceVertex].t * 2];
			objectInfo.vertex[currentVertex + vertexOffset * 3].tex[1]					=	1.0f - mesh->texcoords[mesh->indices[currentFaceVertex].t * 2 + 1];
			objectInfo.vertex[currentVertex + vertexOffset * 3].relMaterialIndex		= mesh->face_materials[currentFace];

			objectInfo.vertex[currentVertex + vertexOffset * 3 + 1].pos[0]				=	mesh->positions[mesh->indices[currentFaceVertex + vertexOffset + 1].p * 3];
			objectInfo.vertex[currentVertex + vertexOffset * 3 + 1].pos[1]				=	mesh->positions[mesh->indices[currentFaceVertex + vertexOffset + 1].p * 3 + 1];
			objectInfo.vertex[currentVertex + vertexOffset * 3 + 1].pos[2]				=	mesh->positions[mesh->indices[currentFaceVertex + vertexOffset + 1].p * 3 + 2] * -1;
			objectInfo.vertex[currentVertex + vertexOffset * 3 + 1].tex[0]				=	mesh->texcoords[mesh->indices[currentFaceVertex + vertexOffset + 1].t * 2];
			objectInfo.vertex[currentVertex + vertexOffset * 3 + 1].tex[1]				=	1.0f - mesh->texcoords[mesh->indices[currentFaceVertex + vertexOffset + 1].t * 2 + 1];
			objectInfo.vertex[currentVertex + vertexOffset * 3 + 1].relMaterialIndex	= mesh->face_materials[currentFace];

			objectInfo.vertex[currentVertex + vertexOffset * 3 + 2].pos[0]				=	mesh->positions[mesh->indices[currentFaceVertex + vertexOffset + 2].p * 3];
			objectInfo.vertex[currentVertex + vertexOffset * 3 + 2].pos[1]				=	mesh->positions[mesh->indices[currentFaceVertex + vertexOffset + 2].p * 3 + 1];
			objectInfo.vertex[currentVertex + vertexOffset * 3 + 2].pos[2]				=	mesh->positions[mesh->indices[currentFaceVertex + vertexOffset + 2].p * 3 + 2] * -1;
			objectInfo.vertex[currentVertex + vertexOffset * 3 + 2].tex[0]				=	mesh->texcoords[mesh->indices[currentFaceVertex + vertexOffset + 2].t * 2];
			objectInfo.vertex[currentVertex + vertexOffset * 3 + 2].tex[1]				=	1.0f - mesh->texcoords[mesh->indices[currentFaceVertex + vertexOffset + 2].t * 2 + 1];
			objectInfo.vertex[currentVertex + vertexOffset * 3 + 2].relMaterialIndex	= mesh->face_materials[currentFace];

			if (1 < mesh->normal_count)
			{
				objectInfo.vertex[currentVertex + vertexOffset * 3].normal[0]		=	mesh->normals[mesh->indices[currentFaceVertex].n * 3];
				objectInfo.vertex[currentVertex + vertexOffset * 3].normal[1]		=	mesh->normals[mesh->indices[currentFaceVertex].n * 3 + 1];
				objectInfo.vertex[currentVertex + vertexOffset * 3].normal[2]		=	mesh->normals[mesh->indices[currentFaceVertex].n * 3 + 2] * -1;

				objectInfo.vertex[currentVertex + vertexOffset * 3 + 1].normal[0]	=	mesh->normals[mesh->indices[currentFaceVertex + vertexOffset + 1].n * 3];
				objectInfo.vertex[currentVertex + vertexOffset * 3 + 1].normal[1]	=	mesh->normals[mesh->indices[currentFaceVertex + vertexOffset + 1].n * 3 + 1];
				objectInfo.vertex[currentVertex + vertexOffset * 3 + 1].normal[2]	=	mesh->normals[mesh->indices[currentFaceVertex + vertexOffset + 1].n * 3 + 2] * -1;

				objectInfo.vertex[currentVertex + vertexOffset * 3 + 2].normal[0]	=	mesh->normals[mesh->indices[currentFaceVertex + vertexOffset + 2].n * 3];
				objectInfo.vertex[currentVertex + vertexOffset * 3 + 2].normal[1]	=	mesh->normals[mesh->indices[currentFaceVertex + vertexOffset + 2].n * 3 + 1];
				objectInfo.vertex[currentVertex + vertexOffset * 3 + 2].normal[2]	=	mesh->normals[mesh->indices[currentFaceVertex + vertexOffset + 2].n * 3 + 2] * -1;
			}
			else
			{
				kdo_findNormal(objectInfo.vertex[currentVertex + vertexOffset * 3].pos, objectInfo.vertex[currentVertex + vertexOffset * 3 + 2].pos, objectInfo.vertex[currentVertex + vertexOffset * 3 + 1].pos, &objectInfo.vertex[currentVertex + vertexOffset * 3].normal);
				glm_vec3_dup(objectInfo.vertex[currentVertex + vertexOffset * 3].normal, objectInfo.vertex[currentVertex + vertexOffset * 3 + 1].normal);
				glm_vec3_dup(objectInfo.vertex[currentVertex + vertexOffset * 3].normal, objectInfo.vertex[currentVertex + vertexOffset * 3 + 2].normal);
			}
		}
		currentVertex		+= 3 * vertexOffset;
		currentFaceVertex	+= 2 + vertexOffset;
	}

	fast_obj_destroy(mesh);
	return (objectInfo);
}
