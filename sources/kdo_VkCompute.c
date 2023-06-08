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
	Kdo_VkObjectDiv *current;
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
		Kdo_VkLoadObjectInfo    info[2];

		info[0].name            = "obj";
		info[0].objectsCount    = 1;
		info[0].texturePath     = "textures/metal.jpg";
		info[0].vertex          = kdo_openObj("obj/alien.obj", &info[0].vertexCount);
		info[0].status          = 0;
		info[0].sampler         = &vk->core.sampler.basic;
		info[1].name            = "obj";
		info[1].objectsCount    = 1;
		info[1].texturePath     = "textures/viking_room.png";
		info[1].vertex          = kdo_openObj("obj/sphere.obj", &info[1].vertexCount);
		info[1].status          = 0;
		info[1].sampler         = &vk->core.sampler.basic;
		kdo_loadObject(vk, &vk->core.objects, 2, info);
		kdo_updateDescripteur(vk, &vk->core.objects);
		start = 0;

		current = kdo_getObject(&vk->core.objects, 0);

		glm_vec3_scale(current->transform->scale, 0.1f, current->transform->scale);
		current->transform->rot[0] = glm_rad(-90.0f * 0);
		kdo_updateTransform(current->transform, 1);
		current = current->next;

		current->transform->pos[2] = 4;
		kdo_updateTransform(current->transform, 1);

	}

	if (0.01 < currentTime - time)
	{
		current = kdo_getObject(&vk->core.objects, 1);

		if ((glfwGetKey(vk->window.path, GLFW_KEY_1)) == GLFW_PRESS)
			current->transform->rot[0] += 0.01f;

		if ((glfwGetKey(vk->window.path, GLFW_KEY_2)) == GLFW_PRESS)
			current->transform->rot[1] += 0.01f;

		if ((glfwGetKey(vk->window.path, GLFW_KEY_3)) == GLFW_PRESS)
			current->transform->rot[2] += 0.01f;
		
		kdo_updateTransform(current->transform + current->count - 1, 1);
		time = currentTime;
	}
}
