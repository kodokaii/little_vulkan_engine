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

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define FAST_OBJ_IMPLEMENTATION
#include "objLoader/fast_obj.h"

static void	kdo_initSampler(Kdo_Vulkan *vk)
{
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
		kdo_cleanup(vk, "Sampler creation failed", 23);
}

static void kdo_initDescriptorPool(Kdo_Vulkan *vk)
{
	VkDescriptorPoolSize			descriptorPoolSize[6];
	VkDescriptorPoolCreateInfo		descriptorPoolInfo;
	VkDescriptorSetAllocateInfo     allocInfo;
	VkDescriptorImageInfo			imageInfo;
	VkWriteDescriptorSet			descriptorWrite;
	char							*defaultTexture = DEFAULT_TEXTURE;

	descriptorPoolSize[0].type				= VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorPoolSize[0].descriptorCount	= 1;

	descriptorPoolSize[1].type				= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorPoolSize[1].descriptorCount	= 1;

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
		kdo_cleanup(vk, "Descriptor pool creation failed", 23);

	allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext                 = NULL;
	allocInfo.descriptorPool        = vk->core.descriptorPool;
	allocInfo.descriptorSetCount    = 1;
	allocInfo.pSetLayouts           = &vk->graphicsPipeline.descriptorLayout;
	if (vkAllocateDescriptorSets(vk->device.path, &allocInfo, &vk->core.descriptorSet) != VK_SUCCESS)
              kdo_cleanup(vk, "Descriptor set allocation failed", 35);

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

	kdo_loadTextures(vk, 1, &defaultTexture);
}

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

static uint32_t	kdo_splitMesh(Kdo_Vertex *vertex, uint32_t *index, uint32_t *sortIndex , uint32_t sortCount)
{
	uint32_t	beforeCount	= 0;
	uint32_t	sameCount	= 1;
	uint32_t	afterCount	= 0;
	uint32_t	buffer;
	float		res;

	while (beforeCount + sameCount + afterCount < sortCount)
	{
		res = kdo_vertexCmp(vertex[sortIndex[beforeCount]], vertex[sortIndex[beforeCount + sameCount]]);

		if (res < 0)
		{
			buffer								= sortIndex[beforeCount];
			sortIndex[beforeCount]				= sortIndex[beforeCount + sameCount];
			sortIndex[beforeCount + sameCount]	= buffer;

			index[sortIndex[beforeCount]]				-= sameCount;
			index[sortIndex[beforeCount + sameCount]]	+= sameCount;

			beforeCount++;
		}
		else if (0 < res)
		{
			buffer									= sortIndex[beforeCount + sameCount];
			sortIndex[beforeCount + sameCount]		= sortIndex[sortCount - afterCount - 1];
			sortIndex[sortCount - afterCount - 1]	= buffer;

			index[sortIndex[beforeCount + sameCount]]		-= sortCount - afterCount - beforeCount - sameCount - 1;
			index[sortIndex[sortCount - afterCount - 1]]	+= sortCount - afterCount - beforeCount - sameCount - 1;

			afterCount++;
		}
		else
			sameCount++;
	}

	if (1 < beforeCount)
		sameCount += kdo_splitMesh(vertex, index, sortIndex, beforeCount);
	if (1 < afterCount)
		sameCount += kdo_splitMesh(vertex, index, sortIndex + sortCount - afterCount, afterCount);

	return (sameCount - 1);
}

static void kdo_findNormal(vec3 vecteur1, vec3 vecteur2, vec3 vecteur3, vec3 *normal)
{
	vec3	x;
	vec3	y;

	glm_vec3_sub(vecteur2, vecteur3, x);
	glm_vec3_sub(vecteur2, vecteur1, y);
	glm_vec3_crossn(x, y, *normal);
}

static void	kdo_initTransform(Kdo_VkTransform *transform)
{
	glm_mat4_identity(transform->modelMat);
	glm_mat4_identity(transform->normalMat);
	glm_vec3_zero(transform->pos);
	glm_vec3_zero(transform->rot);
	glm_vec3_one(transform->scale);
	transform->status = 0;
}

void	kdo_initCore(Kdo_Vulkan *vk)
{
	VkCommandPoolCreateInfo	commandPoolInfo;

	commandPoolInfo.sType               = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext               = NULL;
	commandPoolInfo.flags               = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolInfo.queueFamilyIndex    = vk->device.queues[TRANSFER_QUEUE].familyIndex;
	if (vkCreateCommandPool(vk->device.path, &commandPoolInfo, NULL, &vk->core.transferPool) != VK_SUCCESS)
		kdo_cleanup(vk, "Transfer pool creation failed", 21);

	vk->core.textures.properties.memoryFilter	= findTextureMemoryFiltrer(vk);
    vk->core.textures.properties.layout			= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vk->core.textures.properties.waitFlags		= WAIT_DEVICE;

    vk->core.vertex.properties.usage			= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vk->core.vertex.properties.waitFlags		= WAIT_DEVICE;
	
    vk->core.index.properties.usage				= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vk->core.index.properties.waitFlags			= WAIT_DEVICE;

	vk->core.materials.properties.usage			= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	vk->core.materials.properties.waitFlags		= WAIT_DEVICE;

	vk->core.materialMap.properties.usage		= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	vk->core.materialMap.properties.waitFlags	= WAIT_DEVICE;

	vk->core.light.properties.usage				= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	vk->core.light.properties.waitFlags			= WAIT_DEVICE;

	vk->core.objectMap.properties.usage			= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	vk->core.objectMap.properties.waitFlags		= WAIT_DEVICE;

	vk->core.drawCommand.properties.usage		= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
	vk->core.drawCommand.properties.waitFlags	= WAIT_DEVICE;

	kdo_initSampler(vk);	
	kdo_initDescriptorPool(vk);
}

void	kdo_loadMesh(Kdo_Vulkan *vk, uint32_t infoCount, Kdo_VkLoadMeshInfo *info)
{
	Kdo_VkBuffer		stagingBuffer;
	Kdo_VkLoadDataInfo	*vertexInfo;
	Kdo_VkLoadDataInfo	*indexInfo;
	uint32_t			*sortVertexIndex;
	uint32_t			currentVertex;
	uint32_t			sameVertex;
	uint32_t			i;
	uint32_t			j;

	if (!(vertexInfo = malloc(infoCount * sizeof(Kdo_VkLoadDataInfo))))
		kdo_cleanup(vk, ERRLOC, 12);	
	if (!(indexInfo = malloc(infoCount * sizeof(Kdo_VkLoadDataInfo))))
		kdo_cleanup(vk, ERRLOC, 12);	

	for (i = 0; i < infoCount; i++)
	{
		indexInfo[i].count			= info[i].count;
		indexInfo[i].elementSize	= sizeof(uint32_t);
		if (!(indexInfo[i].data = malloc(indexInfo[i].count * indexInfo[i].elementSize)))
			kdo_cleanup(vk, ERRLOC, 12);

		if (!(sortVertexIndex				= malloc(info[i].count * sizeof (uint32_t))))
			kdo_cleanup(vk, ERRLOC, 12);
		for (j = 0; j < info[i].count; j++)
		{
			((uint32_t *) indexInfo[i].data)[j] = j;
			sortVertexIndex[j]					= j;
		}

		vertexInfo[i].count			= info[i].count - kdo_splitMesh(info[i].vertex, indexInfo[i].data, sortVertexIndex, info[i].count);
		vertexInfo[i].elementSize	= sizeof(Kdo_Vertex);
		if (!(vertexInfo[i].data = malloc(vertexInfo[i].count * vertexInfo[i].elementSize)))
			kdo_cleanup(vk, ERRLOC, 12);

		currentVertex	= 0;
		sameVertex		= 0;
		((Kdo_Vertex *) vertexInfo[i].data)[currentVertex++] = info[i].vertex[sortVertexIndex[0]];
		for (j = 0; j + 1 < info[i].count; j++)
		{
			if (kdo_vertexCmp(info[i].vertex[sortVertexIndex[j]], info[i].vertex[sortVertexIndex[j + 1]]))
				((Kdo_Vertex *) vertexInfo[i].data)[currentVertex++] = info[i].vertex[sortVertexIndex[j + 1]];
			else
				sameVertex++;
			((uint32_t *) indexInfo[i].data)[sortVertexIndex[j + 1]] -= sameVertex;
		}
		free(sortVertexIndex);
	}

	stagingBuffer	= kdo_loadData(vk, infoCount, vertexInfo);
	vk->core.vertex	= kdo_catBuffer(vk, &vk->core.vertex, &stagingBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	stagingBuffer	= kdo_loadData(vk, infoCount, indexInfo);
	vk->core.index	= kdo_catBuffer(vk, &vk->core.index, &stagingBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	for (i = 0; i < infoCount; i++)
	{
		free(indexInfo[i].data);
		free(vertexInfo[i].data);
	}
	free(indexInfo);
	free(vertexInfo);
}

void	kdo_loadTextures(Kdo_Vulkan *vk, uint32_t texturesCount, char **texturesPath)
{
	VkDescriptorImageInfo			*descriptorInfo;
	VkWriteDescriptorSet			descriptorWrite;
	Kdo_VkBuffer					stagingBuffer;
	Kdo_VkLoadDataInfo				*loadInfo;
	Kdo_VkCatBufferToImageInfo		catInfo;
	int								texWidth;
	int								texHeight;
	int								texChannels;
	uint32_t						i;

	if (!(loadInfo		= malloc(texturesCount * sizeof(Kdo_VkLoadDataInfo))))
		kdo_cleanup(vk, ERRLOC, 12);

	catInfo.layout			= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	catInfo.func.imageInfo	= kdo_imageTextureInfo;
	catInfo.func.viewInfo		= kdo_viewTextureInfo;
	catInfo.imagesCount		= texturesCount;
	if (!(catInfo.extents		= malloc(texturesCount * sizeof(Kdo_VkImageDiv))))
		kdo_cleanup(vk, ERRLOC, 12);

	for (i = 0; i < texturesCount; i++)
	{
		if (!(loadInfo[i].data		= stbi_load(texturesPath[i], &texWidth, &texHeight, &texChannels, STBI_rgb_alpha)))
			kdo_cleanup(vk, "Textures load failed", 27);

		loadInfo[i].count		= texWidth * texHeight;
		loadInfo[i].elementSize	= 4;

		catInfo.extents[i].width	= texWidth;
		catInfo.extents[i].height	= texHeight;
		catInfo.extents[i].depth	= 1;
	}

	stagingBuffer		= kdo_loadData(vk, texturesCount, loadInfo);
	vk->core.textures	= kdo_catBufferToImage(vk, &stagingBuffer, &vk->core.textures, catInfo);

	if (!(descriptorInfo = malloc(texturesCount * sizeof(VkDescriptorImageInfo))))
		kdo_cleanup(vk, ERRLOC, 12);

	for (uint32_t i = 0; i < texturesCount; i++)
	{
		descriptorInfo[i].sampler			= NULL;
		descriptorInfo[i].imageView			= vk->core.textures.div[vk->core.textures.divCount - texturesCount + i].view;
		descriptorInfo[i].imageLayout		= vk->core.textures.properties.layout;
	}

	descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext				= NULL;
	descriptorWrite.dstSet				= vk->core.descriptorSet;
	descriptorWrite.dstBinding			= 3;
	descriptorWrite.dstArrayElement		= vk->core.textures.divCount - texturesCount;
	descriptorWrite.descriptorCount		= texturesCount;
	descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorWrite.pImageInfo			= descriptorInfo;
	descriptorWrite.pBufferInfo			= NULL;
	descriptorWrite.pTexelBufferView	= NULL;
	vkUpdateDescriptorSets(vk->device.path, 1, &descriptorWrite, 0, NULL);

	free(descriptorInfo);
	free(loadInfo);
	free(catInfo.extents);
}

void	kdo_loadObject(Kdo_Vulkan *vk, uint32_t objectCount, Kdo_VkObjectInfo *info)
{
	Kdo_VkObjectDiv		*objectDiv;
	Kdo_ShObjectMap     *objectMap;
	uint32_t			*materialMap;
	Kdo_VkLoadMeshInfo	*loadMeshInfo;
	Kdo_VkLoadDataInfo	*loadObjectMapInfo;
	Kdo_VkLoadDataInfo	*loadMaterialInfo;
	Kdo_VkLoadDataInfo	*loadMaterialMapInfo;
	Kdo_VkBuffer		stagingBuffer;
	uint32_t			currentObject;
	uint32_t			currentMaterial;
	uint32_t			materialOffset;
	uint32_t			materialCount;

	materialCount = 0;
	for (currentObject = 0; currentObject < objectCount; currentObject++)
		materialCount += info[currentObject].materialCount;

	if (!(objectDiv				= malloc(objectCount * sizeof(Kdo_VkObjectDiv))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(objectMap				= malloc(objectCount * sizeof(Kdo_ShObjectMap))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(loadObjectMapInfo		= malloc(objectCount * sizeof(Kdo_VkLoadMeshInfo))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(loadMeshInfo			= malloc(objectCount * sizeof(Kdo_VkLoadMeshInfo))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(loadMaterialInfo		= malloc(materialCount * sizeof(Kdo_VkLoadDataInfo))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(loadMaterialMapInfo	= malloc(objectCount * sizeof(Kdo_VkLoadDataInfo))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(materialMap			= malloc(materialCount * sizeof(uint32_t))))
		kdo_cleanup(vk, ERRLOC, 12);

	materialOffset = 0;
	if (0 < vk->core.objects.divCount)
		materialOffset = vk->core.objects.div[vk->core.objects.divCount - 1].materialOffset;
	for (currentObject = 0; currentObject < objectCount; currentObject++)
	{
		for (currentMaterial = 0; currentMaterial < info[currentObject].materialCount; currentMaterial++)
		{
			loadMaterialInfo[materialOffset + currentMaterial].elementSize	= sizeof(Kdo_ShMaterial);
			loadMaterialInfo[materialOffset + currentMaterial].count		= 1;
			loadMaterialInfo[materialOffset + currentMaterial].data			= info[currentObject].material + currentMaterial;
			materialMap[materialOffset + currentMaterial]	= vk->core.materialMap.divCount + materialOffset + currentMaterial;
		}

		loadMeshInfo[currentObject].count	= info[currentObject].vertexCount;
		loadMeshInfo[currentObject].vertex	= info[currentObject].vertex;	

		
		loadMaterialMapInfo[currentObject].elementSize	= sizeof(uint32_t);
		loadMaterialMapInfo[currentObject].count		= info[currentObject].materialCount;
		loadMaterialMapInfo[currentObject].data			= materialMap + materialOffset; 

		glm_mat4_identity(objectMap[currentObject].modelMat);
		glm_mat4_identity(objectMap[currentObject].normalMat);
		glm_rotate_make(objectMap[currentObject].modelMat, glm_rad(-90.0f), GLM_XUP);
		glm_rotate_make(objectMap[currentObject].normalMat, glm_rad(-90.0f), GLM_XUP);
		objectMap[currentObject].materialOffset    = materialOffset;

		loadObjectMapInfo[currentObject].elementSize        = sizeof(Kdo_ShObjectMap);
		loadObjectMapInfo[currentObject].count              = 1;
        loadObjectMapInfo[currentObject].data               = objectMap + currentObject;

		objectDiv[currentObject].name				= info[currentObject].name;
		objectDiv[currentObject].meshIndex			= vk->core.index.divCount + currentObject;
		objectDiv[currentObject].materialOffset		= vk->core.materialMap.divCount + materialOffset;
		kdo_initTransform(&objectDiv[currentObject].transform);

		materialOffset += info[currentObject].materialCount;
	}
	kdo_loadMesh(vk, objectCount, loadMeshInfo);

	stagingBuffer       = kdo_loadData(vk, objectCount, loadObjectMapInfo);
	vk->core.objectMap  = kdo_catBuffer(vk, &vk->core.objectMap, &stagingBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	stagingBuffer = kdo_loadData(vk, materialCount, loadMaterialInfo);
	vk->core.materials = kdo_catBuffer(vk, &vk->core.materials, &stagingBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	stagingBuffer = kdo_loadData(vk, objectCount, loadMaterialMapInfo);
	vk->core.materialMap = kdo_catBuffer(vk, &vk->core.materialMap, &stagingBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vk->core.objects.div		= kdo_mallocMerge(vk->core.objects.divCount * sizeof(Kdo_VkObjectDiv), vk->core.objects.div, objectCount * sizeof(Kdo_VkObjectDiv), objectDiv);
	vk->core.objects.divCount	+= objectCount;

	for (currentObject = 0; currentObject < objectCount; currentObject++)
	{
		free(info[currentObject].vertex);
		free(info[currentObject].material);
	}
	free(objectDiv);
	free(objectMap);
	free(loadObjectMapInfo);
	free(loadMeshInfo);
	free(loadMaterialInfo);
	free(loadMaterialMapInfo);
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

	if (!(objectInfo.vertex		= malloc(objectInfo.vertexCount * sizeof(Kdo_Vertex))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(objectInfo.material	= malloc(objectInfo.materialCount * sizeof(Kdo_ShMaterial))))
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

		objectInfo.material[currentMaterial].map_Kd		= 0;
		objectInfo.material[currentMaterial].map_Ns		= 0;
		objectInfo.material[currentMaterial].map_Bump	= 0;
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

void	kdo_updateTransform(Kdo_VkTransform *transform, uint32_t count)
{
	mat4 translateMat;
	mat4 rotationMat;
	mat4 scaleMat;

	for (uint32_t i = 0; i < count; i++)
	{
		glm_translate_to(GLM_MAT4_IDENTITY, transform[i].pos, translateMat);
		glm_euler(transform[i].rot, rotationMat);
		glm_scale_make(scaleMat, transform[i].scale);

		glm_mat4_mulN((mat4 *[]){&translateMat, &rotationMat, &scaleMat}, 3, transform[i].modelMat);

		scaleMat[0][0] = 1 / scaleMat[0][0];
		scaleMat[1][1] = 1 / scaleMat[1][1];
		scaleMat[2][2] = 1 / scaleMat[2][2];
		glm_mat4_mul(rotationMat, scaleMat, transform[i].normalMat);
	}
}
