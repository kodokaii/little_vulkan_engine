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
	vec3	watch;
	vec3	right;
	vec3	front;
	vec3	top = {0.0f, 0.0f, -1.0f};
	mat4	view;
	mat4	proj;

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
		Kdo_VkLoadObjectInfo    info[1];
		uint32_t	vertexCount;
		Kdo_Vertex	*objVertex = kdo_openObj("obj/furnace.obj", &vertexCount);

		info[0].name            = "obj";
		info[0].objectsCount    = 1;
		info[0].texturePath     = "textures/sky.png";
		info[0].vertexCount     = vertexCount;
		info[0].vertex          = objVertex;
		info[0].status          = 0;
		info[0].sampler         = &vk->core.sampler.basic;
		kdo_loadObject(vk, &vk->core.objects, 1, info);
		kdo_updateDescripteur(vk, &vk->core.objects);
		start = 0;

		Kdo_VkObjectDiv	*current = kdo_getObject(&vk->core.objects, 0);
		glm_vec3_scale(current->model->scale, 0.001f, current->model->scale);
		kdo_updateModel(current->model, 1);
	}

	if (0.01 < currentTime - time)
	{
		Kdo_VkObjectDiv	*current = kdo_getObject(&vk->core.objects, 0);
		//kdo_changeObjectCount(vk, &vk->core.objects, 1, current->count + 1);
	
		//current->model[current->count - 1].pos[2]	= current->model[current->count - 2].pos[2] + 5.5f;
		//current->model[current->count - 1].yaw		= current->model[current->count - 2].yaw + 0.8;
		//glm_vec3_dup(current->model[current->count - 2].scale, current->model[current->count - 1].scale);
		
		current->model[0].yaw += glm_rad(0.1f);

		kdo_updateModel(current->model + current->count - 1, 1);
		time = currentTime;
	}
}
