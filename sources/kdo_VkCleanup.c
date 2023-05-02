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

void	kdo_swapChainCleanup(Kdo_Vulkan *vk)
{
	for (uint32_t i = 0; i < vk->display.maxParallelFrame; i++)
	{
		KDO_DESTROY(vkDestroySemaphore, vk->device.path, vk->semaphore.imageAvailable[i])
		KDO_DESTROY(vkDestroySemaphore, vk->device.path, vk->semaphore.renderFinished[i])
		KDO_DESTROY(vkDestroyFence, vk->device.path, vk->fence.renderFinished[i])
	}
	KDO_DESTROY(vkDestroyRenderPass, vk->device.path, vk->renderPass.path)
	KDO_DESTROY(vkDestroyImageView, vk->device.path, vk->framebuffer.depth.view)
	KDO_DESTROY(vkDestroyImage, vk->device.path, vk->framebuffer.depth.image)
	KDO_DESTROY(vkFreeMemory, vk->device.path, vk->framebuffer.depth.memory)
	for (uint32_t i = 0; i < vk->swapChain.imagesCount; i++)
	{
		KDO_DESTROY(vkDestroyImageView, vk->device.path, vk->swapChain.views[i])
		KDO_DESTROY(vkDestroyFramebuffer, vk->device.path, vk->framebuffer.path[i])
	}
	KDO_DESTROY(vkDestroySwapchainKHR, vk->device.path, vk->swapChain.old)

	KDO_FREE(vk->semaphore.imageAvailable)
	KDO_FREE(vk->semaphore.renderFinished)
	KDO_FREE(vk->fence.renderFinished)
	KDO_FREE(vk->display.frameToImage)
	KDO_FREE(vk->display.imageToFrame)
	KDO_FREE(vk->swapChain.images);
	KDO_FREE(vk->swapChain.views);
	KDO_FREE(vk->framebuffer.path);
}

void	kdo_objectsCleanup(Kdo_Vulkan *vk)
{
	Kdo_VkObject				**currentObject;
	Kdo_VkObject				**nextObject;

	currentObject = &vk->render.objects;
	for (uint32_t i = 0; i < vk->render.objectsCount; i++)
	{
		nextObject = &(*currentObject)->next;
		KDO_FREE((*currentObject)->model)
		KDO_DESTROY(vkDestroyImage, vk->device.path, (*currentObject)->texture)
		KDO_FREE(*currentObject)
		currentObject = nextObject;
	}
}

void	kdo_cleanup(Kdo_Vulkan *vk, char *msg, int returnCode)
{
	printf("%s\nReturn Code: %d\n", msg, returnCode);

	if (vk->device.path)
		vkDeviceWaitIdle(vk->device.path);

	kdo_objectsCleanup(vk);
	KDO_DESTROY(vkDestroyBuffer, vk->device.path, vk->render.vertexAndIndexBuffer)	
	KDO_DESTROY(vkFreeMemory, vk->device.path, vk->render.vertexAndIndexMemory)
	KDO_DESTROY(vkFreeMemory, vk->device.path, vk->render.textureMemory)
	KDO_DESTROY(vkDestroyDescriptorPool, vk->device.path, vk->render.descriptorPool)
	KDO_DESTROY(vkDestroySampler, vk->device.path, vk->render.basicSampler)
	KDO_DESTROY(vkDestroyCommandPool, vk->device.path, vk->render.transferPool)
	KDO_DESTROY(vkDestroyShaderModule, vk->device.path, vk->graphicsPipeline.vertexShader.module)
	KDO_DESTROY(vkDestroyShaderModule, vk->device.path, vk->graphicsPipeline.fragmentShader.module)
	KDO_DESTROY(vkDestroyDescriptorSetLayout, vk->device.path, vk->graphicsPipeline.descriptorLayout)
	KDO_DESTROY(vkDestroyPipelineLayout, vk->device.path, vk->graphicsPipeline.layout)
	KDO_DESTROY(vkDestroyPipeline, vk->device.path, vk->graphicsPipeline.path)
	kdo_swapChainCleanup(vk);
	KDO_DESTROY(vkDestroySwapchainKHR, vk->device.path, vk->swapChain.path)
	KDO_DESTROY(vkDestroySurfaceKHR, vk->instance.path, vk->surface.path)
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
