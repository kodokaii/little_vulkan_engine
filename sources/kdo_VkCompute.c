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
	static uint32_t count		= 1;
	static int		x			= 1;
	static int		y			= 1;
	static int		sign		= 1;
	static int		cp			= 1;

	Kdo_VkObjectDiv			*current;
	Kdo_VkLoadObjectInfo    info[2];
    Kdo_Vertex              vertex[] = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
	};

	if (0.00185f < currentTime - vk->camera.moveTime)
	{
		kdo_updateCamera(vk);
		vk->camera.moveTime	= time;
	}

	if (start)
	{
		info[0].name            = "brick";
		info[0].objectsCount    = 1;
		info[0].texturePath     = "textures/brick.jpg";
		info[0].vertexCount     = 3;
		info[0].vertex          = vertex;
		info[0].status          = 0;
		info[0].sampler         = &vk->core.sampler.basic;
		info[1].name            = "sky";
		info[1].objectsCount    = 1;
		info[1].texturePath     = "textures/sky.png";
		info[1].vertexCount     = 3;
		info[1].vertex          = vertex + 3;
		info[1].status          = 0;
		info[1].sampler         = &vk->core.sampler.basic;
		kdo_loadObject(vk, &vk->core.objects, 2, info);
		kdo_updateDescripteur(vk, &vk->core.objects);
		start = 0;
	}

	if (count < 421 && 0.03 < currentTime - time )
	{

		kdo_changeObjectCount(vk, &vk->core.objects, 0, count + 1);	
		kdo_changeObjectCount(vk, &vk->core.objects, 1, count + 1);	

		current = kdo_getObject(&vk->core.objects, 0);

		
		glm_vec3_dup(current->model[count - 1].pos, current->model[count].pos);
		glm_vec3_dup(current->next->model[count - 1].pos, current->next->model[count].pos);

		if (x)
		{
			current->model[count].pos[0] += sign;
			current->next->model[count].pos[0] += sign;
			x--;
		}
		else if (y)
		{
			current->model[count].pos[1] += sign;
			current->next->model[count].pos[1] += sign;
			y--;
		}

		if (!y)
		{
			cp++;
			x = cp;
			y = cp;
			sign *= -1;
		}

		kdo_updateModel(current->model, count);
		kdo_updateModel(current->next->model, count);
		
		time = currentTime;
		count++;
	}
}
