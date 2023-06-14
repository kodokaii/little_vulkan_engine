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
	VkDescriptorPoolSize			descriptorPoolSize[2];
	VkDescriptorPoolCreateInfo		descriptorPoolInfo;
	VkDescriptorSetAllocateInfo     allocInfo;

	descriptorPoolSize[0].type				= VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorPoolSize[0].descriptorCount	= 1;

	descriptorPoolSize[1].type				= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorPoolSize[1].descriptorCount	= MAX_MATERIAL;

	descriptorPoolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.pNext			= NULL;
	descriptorPoolInfo.flags			= 0;
	descriptorPoolInfo.maxSets			= 1;
	descriptorPoolInfo.poolSizeCount	= 2;
	descriptorPoolInfo.pPoolSizes		= descriptorPoolSize;
	if (vkCreateDescriptorPool(vk->device.path, &descriptorPoolInfo, NULL, &vk->core.descriptorPool) != VK_SUCCESS)
		kdo_cleanup(vk, "Descriptor pool creation failed", 23);

	  allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	  allocInfo.pNext                 = NULL;
	  allocInfo.descriptorPool        = vk->core.descriptorPool;
	  allocInfo.descriptorSetCount    = 1;
	  allocInfo.pSetLayouts           = &vk->graphicsPipeline.descriptorLayout;

	  if (vkAllocateDescriptorSets(vk->device.path, &allocInfo, &vk->core.texturesSet) != VK_SUCCESS)
              kdo_cleanup(vk, "Descriptor set allocation failed", 35);

	VkDescriptorImageInfo	imageInfo;
	VkWriteDescriptorSet	descriptorWrite;

	imageInfo.sampler		= vk->core.sampler.basic;
	imageInfo.imageView		= 0;
	imageInfo.imageLayout	= 0;

	descriptorWrite.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.pNext				= NULL;
	descriptorWrite.dstSet				= vk->core.texturesSet;
	descriptorWrite.dstBinding			= 0;
	descriptorWrite.dstArrayElement		= 0;
	descriptorWrite.descriptorCount		= 1;
	descriptorWrite.descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorWrite.pImageInfo			= &imageInfo;
	descriptorWrite.pBufferInfo			= NULL;
	descriptorWrite.pTexelBufferView	= NULL;
	vkUpdateDescriptorSets(vk->device.path, 1, &descriptorWrite, 0, NULL);
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
	if ((res = kdo_vec3Cmp(vertex1.color, vertex2.color)))
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
	glm_mat4_identity(transform->mat.model);
	glm_mat4_identity(transform->mat.normal);
	glm_vec3_zero(transform->pos);
	glm_vec3_zero(transform->rot);
	glm_vec3_one(transform->scale);
	transform->status = 0;
}

void	kdo_initCore(Kdo_Vulkan *vk)
{
	VkCommandPoolCreateInfo	commandPoolInfo;
	char					*defaultTexture = "textures/default.png";

	commandPoolInfo.sType               = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext               = NULL;
	commandPoolInfo.flags               = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolInfo.queueFamilyIndex    = vk->device.queues[TRANSFER_QUEUE].familyIndex;
	if (vkCreateCommandPool(vk->device.path, &commandPoolInfo, NULL, &vk->core.transferPool) != VK_SUCCESS)
		kdo_cleanup(vk, "Transfer pool creation failed", 21);

	vk->core.textures.properties.memoryFilter	= findTextureMemoryFiltrer(vk);
    vk->core.textures.properties.layout			= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vk->core.textures.properties.waitFlags		= WAIT_DEVICE;
	kdo_loadTextures(vk, 1, &defaultTexture);
    vk->core.vertex.properties.usage			= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vk->core.vertex.properties.waitFlags		= WAIT_DEVICE;
    vk->core.index.properties.usage				= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    vk->core.index.properties.waitFlags			= WAIT_DEVICE;

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
	descriptorWrite.dstSet				= vk->core.texturesSet;
	descriptorWrite.dstBinding			= 1;
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

//TODO
void	kdo_openObj(Kdo_Vulkan *vk, char *objPath, uint32_t *vertexCount)
{
	fastObjMesh			*mesh;
	Kdo_Vertex			*vertex;
	Kdo_VkObjectMap		*objectMap;
	Kdo_VkMaterial		*material;
	Kdo_VkLoadDataInfo	*objectMapLoadInfo;
	Kdo_VkLoadDataInfo	*materialLoadInfo;
	Kdo_VkBuffer        stagingBuffer;
	uint32_t			materialMapCount;
	uint32_t			currentMaterial;
	uint32_t			currentFace;
	uint32_t			currentFaceVertex;
	uint32_t			currentVertex;
	uint32_t			vertexOffset;

	*vertexCount		= 0;
	mesh				= fast_obj_read(objPath);
	for (currentFace	= 0; currentFace < mesh->face_count; currentFace++)
		*vertexCount	+= (mesh->face_vertices[currentFace] - 2);
	*vertexCount		*= 3;

	if (!(vertex		= malloc(*vertexCount * sizeof(Kdo_Vertex))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(objectMap		= malloc(sizeof(Kdo_VkObjectMap) + (mesh->material_count * sizeof(Kdo_VkMaterialMap)))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(material	= malloc(mesh->material_count * sizeof(Kdo_VkMaterial))))
		kdo_cleanup(vk, ERRLOC, 12);

	for (uint32_t currentMaterial  = 0; currentMaterial  < mesh->material_count; currentMaterial ++)
	{
		material[currentMaterial].name	= mesh->materials[currentMaterial].name;
		material[currentMaterial].Ka[0]	= mesh->materials[currentMaterial].Ka[0];
		material[currentMaterial].Ka[1]	= mesh->materials[currentMaterial].Ka[1];
		material[currentMaterial].Ka[2]	= mesh->materials[currentMaterial].Ka[2];
		material[currentMaterial].Kd[0]	= mesh->materials[currentMaterial].Kd[0];
		material[currentMaterial].Kd[1]	= mesh->materials[currentMaterial].Kd[1];
		material[currentMaterial].Kd[2]	= mesh->materials[currentMaterial].Kd[2];
		material[currentMaterial].Ks[0]	= mesh->materials[currentMaterial].Ks[0];
		material[currentMaterial].Ks[1]	= mesh->materials[currentMaterial].Ks[1];
		material[currentMaterial].Ks[2]	= mesh->materials[currentMaterial].Ks[2];
		material[currentMaterial].Ke[0]	= mesh->materials[currentMaterial].Ke[0];
		material[currentMaterial].Ke[1]	= mesh->materials[currentMaterial].Ke[1];
		material[currentMaterial].Ke[2]	= mesh->materials[currentMaterial].Ke[2];
		material[currentMaterial].Kt[0]	= mesh->materials[currentMaterial].Kt[0];
		material[currentMaterial].Kt[1]	= mesh->materials[currentMaterial].Kt[1];
		material[currentMaterial].Kt[2]	= mesh->materials[currentMaterial].Kt[2];
		material[currentMaterial].Ns		= mesh->materials[currentMaterial].Ns;
		material[currentMaterial].Ni		= mesh->materials[currentMaterial].Ni;
		material[currentMaterial].Tf[0]	= mesh->materials[currentMaterial].Tf[0];
		material[currentMaterial].Tf[1]	= mesh->materials[currentMaterial].Tf[1];
		material[currentMaterial].Tf[2]	= mesh->materials[currentMaterial].Tf[2];
		material[currentMaterial].d		= mesh->materials[currentMaterial].d;
		material[currentMaterial].illum	= mesh->materials[currentMaterial].illum;

		material[currentMaterial].texture	= 0;
	}
	objectMap->materialMap[0].materialIndex	= mesh->face_materials[0] + vk->core.materials.divCount;

	currentFaceVertex	= 0;
	currentVertex		= 0;
	currentMaterial		= 0;
	for (currentFace = 0; currentFace < mesh->face_count; currentFace++)
	{
		if (objectMap->materialMap[currentMaterial].materialIndex != mesh->face_materials[currentFace] + vk->core.materials.divCount)
		{
			objectMap->materialMap[currentMaterial].vertexIndex		= currentVertex;
			if (mesh->material_count <= ++currentMaterial)
				if (!(objectMap	= realloc(objectMap, sizeof(Kdo_VkObjectMap) + ((currentMaterial + 1) * sizeof(Kdo_VkMaterialMap)))))
					kdo_cleanup(vk, ERRLOC, 12);
			materialMap[currentMaterial].materialIndex	= mesh->face_materials[currentFace] + vk->core.materials.divCount;
		}
		for (vertexOffset = 0; 3 <= mesh->face_vertices[currentFace] - vertexOffset; vertexOffset++)
		{
			vertex[currentVertex + vertexOffset * 3].pos[0]		=	mesh->positions[mesh->indices[currentFaceVertex].p * 3];
			vertex[currentVertex + vertexOffset * 3].pos[1]		=	mesh->positions[mesh->indices[currentFaceVertex].p * 3 + 1];
			vertex[currentVertex + vertexOffset * 3].pos[2]		=	mesh->positions[mesh->indices[currentFaceVertex].p * 3 + 2] * -1;
			vertex[currentVertex + vertexOffset * 3].tex[0]		=	mesh->texcoords[mesh->indices[currentFaceVertex].t * 2];
			vertex[currentVertex + vertexOffset * 3].tex[1]		=	1.0f - mesh->texcoords[mesh->indices[currentFaceVertex].t * 2 + 1];
			glm_vec3_dup(GLM_VEC3_ONE, vertex[currentVertex + vertexOffset * 3].color);

			vertex[currentVertex + vertexOffset * 3 + 1].pos[0]	=	mesh->positions[mesh->indices[currentFaceVertex + vertexOffset + 1].p * 3];
			vertex[currentVertex + vertexOffset * 3 + 1].pos[1]	=	mesh->positions[mesh->indices[currentFaceVertex + vertexOffset + 1].p * 3 + 1];
			vertex[currentVertex + vertexOffset * 3 + 1].pos[2]	=	mesh->positions[mesh->indices[currentFaceVertex + vertexOffset + 1].p * 3 + 2] * -1;
			vertex[currentVertex + vertexOffset * 3 + 1].tex[0]	=	mesh->texcoords[mesh->indices[currentFaceVertex + vertexOffset + 1].t * 2];
			vertex[currentVertex + vertexOffset * 3 + 1].tex[1]	=	1.0f - mesh->texcoords[mesh->indices[currentFaceVertex + vertexOffset + 1].t * 2 + 1];
			glm_vec3_dup(GLM_VEC3_ONE, vertex[currentVertex + vertexOffset * 3 + 1].color);

			vertex[currentVertex + vertexOffset * 3 + 2].pos[0]	=	mesh->positions[mesh->indices[currentFaceVertex + vertexOffset + 2].p * 3];
			vertex[currentVertex + vertexOffset * 3 + 2].pos[1]	=	mesh->positions[mesh->indices[currentFaceVertex + vertexOffset + 2].p * 3 + 1];
			vertex[currentVertex + vertexOffset * 3 + 2].pos[2]	=	mesh->positions[mesh->indices[currentFaceVertex + vertexOffset + 2].p * 3 + 2] * -1;
			vertex[currentVertex + vertexOffset * 3 + 2].tex[0]	=	mesh->texcoords[mesh->indices[currentFaceVertex + vertexOffset + 2].t * 2];
			vertex[currentVertex + vertexOffset * 3 + 2].tex[1]	=	1.0f - mesh->texcoords[mesh->indices[currentFaceVertex + vertexOffset + 2].t * 2 + 1];
			glm_vec3_dup(GLM_VEC3_ONE, vertex[currentVertex + vertexOffset * 3 + 2].color);

			if (1 < mesh->normal_count)
			{
				vertex[currentVertex + vertexOffset * 3].normal[0]		=	mesh->normals[mesh->indices[currentFaceVertex].n * 3];
				vertex[currentVertex + vertexOffset * 3].normal[1]		=	mesh->normals[mesh->indices[currentFaceVertex].n * 3 + 1];
				vertex[currentVertex + vertexOffset * 3].normal[2]		=	mesh->normals[mesh->indices[currentFaceVertex].n * 3 + 2] * -1;

				vertex[currentVertex + vertexOffset * 3 + 1].normal[0]	=	mesh->normals[mesh->indices[currentFaceVertex + vertexOffset + 1].n * 3];
				vertex[currentVertex + vertexOffset * 3 + 1].normal[1]	=	mesh->normals[mesh->indices[currentFaceVertex + vertexOffset + 1].n * 3 + 1];
				vertex[currentVertex + vertexOffset * 3 + 1].normal[2]	=	mesh->normals[mesh->indices[currentFaceVertex + vertexOffset + 1].n * 3 + 2] * -1;

				vertex[currentVertex + vertexOffset * 3 + 2].normal[0]	=	mesh->normals[mesh->indices[currentFaceVertex + vertexOffset + 2].n * 3];
				vertex[currentVertex + vertexOffset * 3 + 2].normal[1]	=	mesh->normals[mesh->indices[currentFaceVertex + vertexOffset + 2].n * 3 + 1];
				vertex[currentVertex + vertexOffset * 3 + 2].normal[2]	=	mesh->normals[mesh->indices[currentFaceVertex + vertexOffset + 2].n * 3 + 2] * -1;
			}
			else
			{
				kdo_findNormal(vertex[currentVertex + vertexOffset * 3].pos, vertex[currentVertex + vertexOffset * 3 + 2].pos, vertex[currentVertex + vertexOffset * 3 + 1].pos, &vertex[currentVertex + vertexOffset * 3].normal);
				glm_vec3_dup(vertex[currentVertex + vertexOffset * 3].normal, vertex[currentVertex + vertexOffset * 3 + 1].normal);
				glm_vec3_dup(vertex[currentVertex + vertexOffset * 3].normal, vertex[currentVertex + vertexOffset * 3 + 2].normal);
			}
		}
		currentVertex		+= 3 * vertexOffset;
		currentFaceVertex	+= 2 + vertexOffset;
	}
	materialMap[currentMaterial].vertexIndex	= currentVertex;
	materialMapCount							= currentMaterial + 1;

	if (!(objectMapLoadInfo	= malloc(materialMapCount * sizeof(Kdo_VkLoadDataInfo))))
		kdo_cleanup(vk, ERRLOC, 12);
	if (!(materialLoadInfo		= malloc(mesh->material_count * sizeof(Kdo_VkLoadDataInfo))))
		kdo_cleanup(vk, ERRLOC, 12);
	
	for (currentMaterial = 0; currentMaterial < materialMapCount; currentMaterial++)
	{
		objectMapLoadInfo[currentMaterial].elementSize	= sizeof(Kdo_VkObjectMap);
		objectMapLoadInfo[currentMaterial].count		= 1;
		objectMapLoadInfo[currentMaterial].data			= &materialMap[currentMaterial];
	}

	for (currentMaterial = 0; currentMaterial < mesh->material_count; currentMaterial++)
	{
		materialLoadInfo[currentMaterial].elementSize	= sizeof(Kdo_VkMaterial);
		materialLoadInfo[currentMaterial].count			= 1;
		materialLoadInfo[currentMaterial].data			= &material[currentMaterial];
	}

	stagingBuffer			= kdo_loadData(vk, mesh->material_count, materialLoadInfo);
	vk->core.materials		= kdo_catBuffer(vk, &vk->core.materials, &stagingBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	fast_obj_destroy(mesh);
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

		glm_mat4_mulN((mat4 *[]){&translateMat, &rotationMat, &scaleMat}, 3, transform[i].mat.model);

		scaleMat[0][0] = 1 / scaleMat[0][0];
		scaleMat[1][1] = 1 / scaleMat[1][1];
		scaleMat[2][2] = 1 / scaleMat[2][2];
		glm_mat4_mul(rotationMat, scaleMat, transform[i].mat.normal);
	}
}
