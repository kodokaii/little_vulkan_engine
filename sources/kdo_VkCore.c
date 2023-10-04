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

static void kdo_findNormal(vec3 vertice1, vec3 vertice2, vec3 vertice3, vec3 *normal)
{
	vec3	x;
	vec3	y;

	glm_vec3_sub(vertice2, vertice3, x);
	glm_vec3_sub(vertice2, vertice1, y);
	glm_vec3_cross(x, y, *normal);
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
	vkUpdateDescriptorSets(vk->device.path, 1, &descriptorWrite, 0, NULL);
}

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

void	kdo_initCore(Kdo_Vulkan *vk)
{
	VkCommandPoolCreateInfo	commandPoolInfo;

	commandPoolInfo.sType               = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext               = NULL;
	commandPoolInfo.flags               = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
	commandPoolInfo.queueFamilyIndex    = vk->device.queues[TRANSFER_QUEUE].familyIndex;
	if (vkCreateCommandPool(vk->device.path, &commandPoolInfo, NULL, &vk->core.transferPool) != VK_SUCCESS)

		kdo_cleanup(vk, "Transfer pool creation failed", 22);
	
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

Kdo_VkObjectInfo	kdo_openObj(Kdo_Vulkan *vk, char *objPath)
{
	fastObjMesh			*mesh;
	Kdo_VkObjectInfo	objectInfo;
	uint32_t			currentFace;
	uint32_t			currentFaceVertex;
	uint32_t			currentVertex;
	uint32_t			vertexOffset;


	mesh = fast_obj_read(objPath);
	
	if (mesh->material_count)
	{
		objectInfo.materialCount	= mesh->material_count;
		objectInfo.textureCount		= mesh->material_count * 3;

		if (!(objectInfo.material		= malloc(objectInfo.materialCount * sizeof(Kdo_ShMaterial))))
			kdo_cleanup(vk, ERRLOC, 12);
		if (!(objectInfo.texturePath	= calloc(objectInfo.textureCount, sizeof(char *))))
			kdo_cleanup(vk, ERRLOC, 12);

		for (uint32_t currentMaterial  = 0; currentMaterial  < mesh->material_count; currentMaterial ++)
		{
			glm_vec3_make(mesh->materials[currentMaterial].Ka, objectInfo.material[currentMaterial].ambient);
			glm_vec3_make(mesh->materials[currentMaterial].Kd, objectInfo.material[currentMaterial].diffuse);
			glm_vec3_make(mesh->materials[currentMaterial].Ks, objectInfo.material[currentMaterial].specular);
			glm_vec3_make(mesh->materials[currentMaterial].Ke, objectInfo.material[currentMaterial].emissive);
			glm_vec3_make(mesh->materials[currentMaterial].Kt, objectInfo.material[currentMaterial].transmittance);
			glm_vec3_make(mesh->materials[currentMaterial].Tf, objectInfo.material[currentMaterial].transmissionFilter);
			objectInfo.material[currentMaterial].shininess			= mesh->materials[currentMaterial].Ns;
			objectInfo.material[currentMaterial].refractionIndex	= mesh->materials[currentMaterial].Ni;
			objectInfo.material[currentMaterial].disolve			= mesh->materials[currentMaterial].d;
			objectInfo.material[currentMaterial].illum				= mesh->materials[currentMaterial].illum;

			objectInfo.material[currentMaterial].diffuseMap		= currentMaterial * 3;
			objectInfo.material[currentMaterial].specularMap	= currentMaterial * 3 + 1;
			objectInfo.material[currentMaterial].bumpMap		= currentMaterial * 3 + 2;

			if (mesh->materials[currentMaterial].map_Kd.path)
				if (!(objectInfo.texturePath[currentMaterial * 3]		= strdup(mesh->materials[currentMaterial].map_Kd.path)))
					kdo_cleanup(vk, ERRLOC, 12);

			if (mesh->materials[currentMaterial].map_Ks.path)
				if (!(objectInfo.texturePath[currentMaterial * 3 + 1]	= strdup(mesh->materials[currentMaterial].map_Ks.path)))
					kdo_cleanup(vk, ERRLOC, 12);

			if (mesh->materials[currentMaterial].map_bump.path)
				if (!(objectInfo.texturePath[currentMaterial * 3 + 2]	= strdup(mesh->materials[currentMaterial].map_bump.path)))
					kdo_cleanup(vk, ERRLOC, 12);
		}
	}
	else
	{
		objectInfo.materialCount	= 1;
		objectInfo.textureCount		= 1;

		if (!(objectInfo.texturePath	= malloc(sizeof(Kdo_ShMaterial))))
			kdo_cleanup(vk, ERRLOC, 12);
		if (!(objectInfo.material		= malloc(sizeof(Kdo_ShMaterial))))
			kdo_cleanup(vk, ERRLOC, 12);

		glm_vec3_broadcast(0, objectInfo.material[0].ambient);
		glm_vec3_broadcast(1, objectInfo.material[0].diffuse);
		glm_vec3_broadcast(1, objectInfo.material[0].specular);
		glm_vec3_broadcast(1, objectInfo.material[0].emissive);
		glm_vec3_broadcast(1, objectInfo.material[0].transmittance);
		glm_vec3_broadcast(1, objectInfo.material[0].transmissionFilter);
		objectInfo.material[0].shininess		= 400;
		objectInfo.material[0].refractionIndex	= 1;
		objectInfo.material[0].disolve			= 1;
		objectInfo.material[0].illum			= 0;

		objectInfo.material[0].diffuseMap		= 0;
		objectInfo.material[0].specularMap		= 0;
		objectInfo.material[0].bumpMap			= 0;

		objectInfo.texturePath[0] = NULL;
	}


	currentFaceVertex		= 0;
	currentVertex			= 0;
	objectInfo.vertexCount	= 0;
	for (currentFace = 0; currentFace < mesh->face_count; currentFace++)
		objectInfo.vertexCount	+= (mesh->face_vertices[currentFace] - 2);
	objectInfo.vertexCount	*= 3;

	if (!(objectInfo.vertex			= malloc(objectInfo.vertexCount * sizeof(Kdo_Vertex))))
		kdo_cleanup(vk, ERRLOC, 12);

	for(currentFace = 0; currentFace < mesh->face_count; currentFace++)
	{
	}

	fast_obj_destroy(mesh);
	return (objectInfo);
}
