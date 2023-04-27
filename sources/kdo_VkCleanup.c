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

#include "kdo_VkCleanup.h"


void	kdo_freePhysicalDevice(Kdo_Vulkan *vk)
{
	KDO_FREE(vk->physicalDevice.swapChainProperties.formats)
	KDO_FREE(vk->physicalDevice.swapChainProperties.presentModes)
	KDO_FREE(vk->physicalDevice.queueProperties.path)
	KDO_FREE(vk->physicalDevice.queueProperties.presentSupport)
}

void	kdo_swapchainCleanup(Kdo_Vulkan *vk)
{

}

void	kdo_cleanup(Kdo_Vulkan *vk, char *msg, int returnCode)
{
	printf("%s\nReturn Code: %d\n", msg, returnCode);

	if (vk->device.path)
	{
		vkDestroyDevice(vk->device.path, NULL);
		vk->device.path = NULL;
	}
	KDO_DESTROY(vkDestroySurfaceKHR, vk->instance.path, vk->surface.path)
	if (vk->instance.path)
	{
		vkDestroyInstance(vk->instance.path, NULL);
		vk->instance.path = NULL;
	}
	
	kdo_freePhysicalDevice(vk);
	KDO_FREE(vk->info.instanceExtensions)

	glfwTerminate();

	exit(returnCode);
}
