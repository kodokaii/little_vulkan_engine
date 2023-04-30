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

#include "kdo_VkSyncronisation.h"

void	kdo_initSyncronisation(Kdo_Vulkan *vk)
{
	VkSemaphoreCreateInfo	semaphoreInfo;
	VkFenceCreateInfo		fenceInfo;

	if ((vk->display.maxParallelFrame = vk->swapChain.imagesCount - 1) < 1)
		vk->display.maxParallelFrame = 1;

	if (!(vk->semaphore.imageAvailable = malloc(vk->display.maxParallelFrame * sizeof(VkSemaphore))))
		kdo_cleanup(vk, ERRLOC, 12);

	if (!(vk->semaphore.renderFinished = malloc(vk->display.maxParallelFrame * sizeof(VkSemaphore))))
		kdo_cleanup(vk, ERRLOC, 12);

	if (!(vk->fence.renderFinished = malloc(vk->display.maxParallelFrame * sizeof(VkFence))))
		kdo_cleanup(vk, ERRLOC, 12);

	if (!(vk->display.frameToImage = calloc(vk->display.maxParallelFrame, sizeof(Kdo_SynImage))))
		kdo_cleanup(vk, ERRLOC, 12);

	if (!(vk->display.imageToFrame = calloc(vk->swapChain.imagesCount, sizeof(Kdo_SynImage *))))
		kdo_cleanup(vk, ERRLOC, 12);


	for (uint32_t i = 0; i < vk->display.maxParallelFrame; i++)
	{
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreInfo.pNext	= NULL;
		semaphoreInfo.flags	= 0;
		if (vkCreateSemaphore(vk->device.path, &semaphoreInfo, NULL, vk->semaphore.imageAvailable + i) != VK_SUCCESS)
			kdo_cleanup(vk, "Semaphore creation failed", 20);
		if (vkCreateSemaphore(vk->device.path, &semaphoreInfo, NULL, vk->semaphore.renderFinished + i) != VK_SUCCESS)
			kdo_cleanup(vk, "Semaphore creation failed", 20);

		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.pNext	= NULL;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		if (vkCreateFence(vk->device.path, &fenceInfo, NULL, vk->fence.renderFinished + i) != VK_SUCCESS)
			kdo_cleanup(vk, "Fence creation failed", 21);

		vk->display.frameToImage[i].renderFinishedFence	= vk->fence.renderFinished + i;
	}
}
