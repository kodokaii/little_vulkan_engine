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

#include "kdo_VkGlfw.h"
#include "kdo_VkInit.h"
#include "kdo_VkDisplay.h"
#include "kdo_VkObject.h"

int	main(int argc, char *argv[])
{
	Kdo_Vulkan		vk						= {};
	const char		*validationLayers[]		= {"VK_LAYER_KHRONOS_validation"};
	const char		*deviceExtensions[]		= {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	Kdo_VkLoadMeshInfo	info[3];
	Kdo_Vertex			vertex[] = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
};

	(void) argc;
	(void) argv;

	vk.info.applicationName						= "Vulkan Project";
	vk.info.engineName							= "Kodo Engine";
	vk.info.instanceExtensionsCount				= 0;
	vk.info.instanceExtensions					= NULL;
	vk.info.validationLayersCount				= 1;
	vk.info.validationLayers					= validationLayers;
	vk.info.deviceExtensionsCount				= 1;
	vk.info.deviceExtensions					= deviceExtensions;
	vk.info.windowWidth							= 1000;
	vk.info.windowHeight						= 1000;

	vk.info.queuesInfo[PRESENT_QUEUE].count				= 1;
	vk.info.queuesInfo[PRESENT_QUEUE].priority			= 1.0f;
	vk.info.queuesInfo[PRESENT_QUEUE].requiredFlags		= 0; 
	vk.info.queuesInfo[PRESENT_QUEUE].noRequiredFlags	= 0;
	vk.info.queuesInfo[PRESENT_QUEUE].preferredFlags	= VK_QUEUE_GRAPHICS_BIT;
	vk.info.queuesInfo[PRESENT_QUEUE].noPreferredFlags	= 0;
	vk.info.queuesInfo[PRESENT_QUEUE].presentSupport	= VK_TRUE;

	vk.info.queuesInfo[GRAPHIC_QUEUE].count				= 1;
	vk.info.queuesInfo[GRAPHIC_QUEUE].priority			= 1.0f;
	vk.info.queuesInfo[GRAPHIC_QUEUE].requiredFlags		= VK_QUEUE_GRAPHICS_BIT;
	vk.info.queuesInfo[GRAPHIC_QUEUE].noRequiredFlags	= 0;
	vk.info.queuesInfo[GRAPHIC_QUEUE].preferredFlags	= 0;
	vk.info.queuesInfo[GRAPHIC_QUEUE].noPreferredFlags	= 0;
	vk.info.queuesInfo[GRAPHIC_QUEUE].presentSupport	= 0;

	vk.info.queuesInfo[TRANSFER_QUEUE].count			= 1;
	vk.info.queuesInfo[TRANSFER_QUEUE].priority			= 1.0f;
	vk.info.queuesInfo[TRANSFER_QUEUE].requiredFlags	= VK_QUEUE_GRAPHICS_BIT;
	vk.info.queuesInfo[TRANSFER_QUEUE].noRequiredFlags	= 0;
	vk.info.queuesInfo[TRANSFER_QUEUE].preferredFlags	= 0;
	vk.info.queuesInfo[TRANSFER_QUEUE].noPreferredFlags	= 0;
	vk.info.queuesInfo[TRANSFER_QUEUE].presentSupport	= 0;

	vk.info.presentMode							= VK_PRESENT_MODE_MAILBOX_KHR; 
	vk.info.startPos[0]							= 4.0f;
	vk.info.startPos[1]							= 0.0f;
	vk.info.startPos[2]							= -1.0f;
	vk.info.startYaw							= 0.0f;
	vk.info.startPitch							= 0.0f;

	kdo_initGlfw(&vk);
	kdo_initVulkan(&vk);

	vk.render.vertex.usage	= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vk.render.index.usage	= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	
	info[0].count			= 6;
	info[0].vertex			= vertex;
	info[1].count			= 6;
	info[1].vertex			= vertex;
	info[2].count			= 6;
	info[2].vertex			= vertex;
	kdo_loadMesh(&vk, &vk.render.vertex, &vk.render.index, 1, info);
	kdo_loadMesh(&vk, &vk.render.vertex, &vk.render.index, 3, info);
	kdo_loadMesh(&vk, &vk.render.vertex, &vk.render.index, 2, info);

	kdo_cleanup(&vk, "Good !", 0);
}
