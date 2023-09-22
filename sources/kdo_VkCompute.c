/* *********************************************************************************** */
/*       :::    ::: ::::::::  :::::::::   ::::::::  :::    :::     :::     ::::::::::: */
/*      :+:   :+: :+:    :+: :+:    :+: :+:    :+: :+:   :+:    :+: :+:       :+:      */
/*     +:+  +:+  +:+    +:+ +:+    +:+ +:+    +:+ +:+  +:+    +:+   +:+      +:+       */
/*    +#++:++   +#+    +:+ +#+    +:+ +#+    +:+ +#++:++    +#++:++#++:     +#+        */ /*   +#+  +#+  +#+    +#+ +#+    +#+ +#+    +#+ +#+  +#+   +#+     +#+     +#+         */ /*  #+#   #+# #+#    #+# #+#    #+# #+#    #+# #+#   #+#  #+#     #+#     #+#          */
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

void	kdo_createMat(mat4 modelMat, mat4 normalMat, vec3 translate, vec3 rotation, vec3 scale)
{
	mat4 translateMat;
	mat4 rotationMat;
	mat4 scaleMat;

	glm_translate_to(GLM_MAT4_IDENTITY, translate, translateMat);
	glm_euler(rotation, rotationMat);
	glm_scale_make(scaleMat, scale);

	glm_mat4_mulN((mat4 *[]){&translateMat, &rotationMat, &scaleMat}, 3, modelMat);

	scaleMat[0][0] = 1 / scaleMat[0][0];
	scaleMat[1][1] = 1 / scaleMat[1][1];
	scaleMat[2][2] = 1 / scaleMat[2][2];
	glm_mat4_mul(rotationMat, scaleMat, normalMat);
}

void kdo_compute(Kdo_Vulkan *vk)
{
	vk->compute.currentTime	= glfwGetTime();

	kdo_updateCamera(vk);

	if (vk->compute.startTime == 0)
	{
		Kdo_ShLight			light			= {{5.0f, 0.0f, -10.0f, 10.0f}, {1.0f, 1.0f, 1.0f, 1.0f}};
		Kdo_VkObjectInfo	objectInfo[1]	= {kdo_openObj(vk, "obj/aircraft/aircraft.obj")};
		mat4				mat[2]			= {GLM_MAT4_IDENTITY_INIT, GLM_MAT4_IDENTITY_INIT};
		vec3				vec;

		kdo_setData(vk, &vk->core.buffer.light, &light, sizeof(Kdo_ShLight), vk->core.buffer.light.sizeUsed);

		kdo_loadObject(vk, objectInfo[0]);
		printf("triangleCount = %d", vk->core.count.index / 3);

		glm_vec3_broadcast(1, vec);
		kdo_createMat(mat[0], mat[1], GLM_VEC3_ZERO, GLM_VEC3_ZERO, vec);
		kdo_setData(vk, &vk->core.buffer.object, mat, 2 * sizeof(mat4), 0);

		vk->compute.startTime = vk->compute.currentTime;
	}

	vk->compute.fps++;
	if (1 < vk->compute.currentTime - vk->compute.secondTime)
	{
		printf("%d\n", vk->compute.fps);
		vk->compute.fps = 0;
		vk->compute.secondTime = vk->compute.currentTime;
	}
}
