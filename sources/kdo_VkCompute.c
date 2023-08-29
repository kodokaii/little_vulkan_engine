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

#include "kdo_VkCompute.h"

static void	kdo_updateCamera(Kdo_Vulkan *vk)
{
	float	speed		= 0.05f;
	float	sensitivity	= glm_rad(0.15f);
	double	yCurrentMousse;
	double	xCurrentMousse;
	mat4	view;
	mat4	proj;
	vec3	watch;
	vec3	right;
	vec3	front;
	vec3	top = {0.0f, 0.0f, -1.0f};

	glfwGetCursorPos(vk->window.path, &xCurrentMousse, &yCurrentMousse);
	vk->camera.yaw	= vk->camera.yaw + ((xCurrentMousse - vk->camera.xMouse) * sensitivity);
	vk->camera.pitch	= glm_clamp(vk->camera.pitch + ((yCurrentMousse - vk->camera.yMouse) * sensitivity), glm_rad(-89.99), glm_rad(89.99));
	watch[0] = cos(vk->camera.pitch) * cos(vk->camera.yaw);
	watch[1] = cos(vk->camera.pitch) * sin(vk->camera.yaw);
	watch[2] = sin(vk->camera.pitch);

	glm_vec3_crossn(watch, top, right);
	glm_vec3_crossn(top, right, front);

	glm_vec3_scale(top, speed, top);
	glm_vec3_scale(right, speed, right);
	glm_vec3_scale(front, speed, front);

	if ((glfwGetKey(vk->window.path, GLFW_KEY_W)) == GLFW_PRESS)
		glm_vec3_add(vk->camera.pos, front, vk->camera.pos);

	if ((glfwGetKey(vk->window.path, GLFW_KEY_S)) == GLFW_PRESS)
		glm_vec3_sub(vk->camera.pos, front, vk->camera.pos);

	if ((glfwGetKey(vk->window.path, GLFW_KEY_D)) == GLFW_PRESS)
		glm_vec3_add(vk->camera.pos, right, vk->camera.pos);

	if ((glfwGetKey(vk->window.path, GLFW_KEY_A)) == GLFW_PRESS)
		glm_vec3_sub(vk->camera.pos, right, vk->camera.pos);

	if ((glfwGetKey(vk->window.path, GLFW_KEY_SPACE)) == GLFW_PRESS)
		glm_vec3_add(vk->camera.pos, top, vk->camera.pos);

	if ((glfwGetKey(vk->window.path, GLFW_KEY_LEFT_CONTROL)) == GLFW_PRESS)
		glm_vec3_sub(vk->camera.pos, top, vk->camera.pos);
	
	glm_look(vk->camera.pos, watch, GLM_ZUP, view);
	glm_perspective(glm_rad(60.0f), vk->swapChain.imagesExtent.width / vk->swapChain.imagesExtent.height, 0.1f, 50.0f, proj);

	glm_mat4_mul(proj, view, vk->camera.path);

	vk->camera.xMouse = xCurrentMousse;
	vk->camera.yMouse = yCurrentMousse;
}

void kdo_compute(Kdo_Vulkan *vk)
{
	double			currentTime	= glfwGetTime();
	static double	time		= 0;
	static int		start		= 1;

	if (0.00185f < currentTime - vk->camera.moveTime)
	{
		kdo_updateCamera(vk);
		vk->camera.moveTime	= currentTime;
	}

	if (start)
	{
		VkDrawIndexedIndirectCommand	drawCommand;
		Kdo_ShLight						light = {{5.0f, 0.0f, -10.0f, 10.0f}, {1.0f, 1.0f, 1.0f, 1.0f}};
		Kdo_VkLoadDataInfo				loadInfo;
		Kdo_VkObjectInfo				objectInfo = kdo_openObj(vk, "obj/bugatti.obj");
		Kdo_VkBuffer					stagingBuffer;
		VkWriteDescriptorSet			descriptorWrite[4];
		VkDescriptorBufferInfo			writeBufferInfo[4];

		objectInfo.name = "bugatti";
		kdo_loadObject(vk, 1, &objectInfo);

		drawCommand.indexCount		= vk->core.index.div->count;
		drawCommand.instanceCount	= 1;
		drawCommand.firstIndex		= 0;
		drawCommand.vertexOffset	= 0;
		drawCommand.firstInstance	= 0;

		loadInfo.elementSize		= sizeof(VkDrawIndexedIndirectCommand);
		loadInfo.count				= 1;
		loadInfo.data				= &drawCommand;
		stagingBuffer			= kdo_loadData(vk, 1, &loadInfo);
		vk->core.drawCommand	= kdo_catBuffer(vk, &vk->core.drawCommand, &stagingBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		loadInfo.elementSize		= sizeof(Kdo_ShLight);
		loadInfo.count				= 1;
		loadInfo.data				= &light;
		stagingBuffer		= kdo_loadData(vk, 1, &loadInfo);
		vk->core.light		= kdo_catBuffer(vk, &vk->core.light, &stagingBuffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		writeBufferInfo[0].buffer	= vk->core.objectMap.buffer;
		writeBufferInfo[0].offset	= 0;
		writeBufferInfo[0].range	= vk->core.objectMap.size;

		writeBufferInfo[1].buffer	= vk->core.materialMap.buffer;
		writeBufferInfo[1].offset	= 0;
		writeBufferInfo[1].range	= vk->core.materialMap.size;

		writeBufferInfo[2].buffer	= vk->core.light.buffer;
		writeBufferInfo[2].offset	= 0;
		writeBufferInfo[2].range	= vk->core.light.size;

		writeBufferInfo[3].buffer	= vk->core.materials.buffer;
		writeBufferInfo[3].offset	= 0;
		writeBufferInfo[3].range	= vk->core.materials.size;


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

		start = 0;
	}

	if (0.01 < currentTime - time)
	{
		time = currentTime;
	}
}
