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

#define FAST_OBJ_IMPLEMENTATION
#include "objLoader/fast_obj.h"

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
		vk->camera.moveTime	= currentTime;
	}

	if (start)
	{
		fastObjMesh* mesh = fast_obj_read("obj/pyramid.obj");

		Kdo_Vertex *test = malloc(mesh->index_count * sizeof(Kdo_Vertex));

		for(uint32_t i = 0; i < mesh->index_count; i++)
		{
			test[i].pos[0]	=	mesh->positions[mesh->indices[i].p * 3];
			test[i].pos[1]	=	mesh->positions[mesh->indices[i].p * 3 + 1];
			test[i].pos[2]	=	mesh->positions[mesh->indices[i].p * 3 + 2] * -1;
			test[i].tex[0]	=	mesh->texcoords[mesh->indices[i].t * 2];
			test[i].tex[1]	=	1.0f - mesh->texcoords[mesh->indices[i].t * 2 + 1];
			glm_vec3_dup(GLM_VEC3_ONE, test[i].color);
		}

		info[0].name            = "brick";
		info[0].objectsCount    = 1;
		info[0].texturePath     = "textures/brick.jpg";
		info[0].vertexCount     = 6;
		info[0].vertex          = vertex;
		info[0].status          = 0;
		info[0].sampler         = &vk->core.sampler.basic;
		info[1].name            = "house";
		info[1].objectsCount    = 1;
		info[1].texturePath     = "textures/viking_room.png";
		info[1].vertexCount     = mesh->index_count;
		info[1].vertex          = test;
		info[1].status          = 0;
		info[1].sampler         = &vk->core.sampler.basic;
		kdo_loadObject(vk, &vk->core.objects, 2, info);
		kdo_updateDescripteur(vk, &vk->core.objects);
		start = 0;
	}



	if (1 < currentTime - time)
	{
		Kdo_VkObjectDiv	*current = kdo_getObject(&vk->core.objects, 1);
		kdo_changeObjectCount(vk, &vk->core.objects, 1, current->count + 1);
	
		current->model[current->count - 1].pos[2] = current->model[current->count - 2].pos[2] + 1.5f;
		current->model[current->count - 1].yaw = current->model[current->count - 2].yaw + 0.2;

		kdo_updateModel(current->model + current->count - 1, 1);
		time = currentTime;
	}

}
