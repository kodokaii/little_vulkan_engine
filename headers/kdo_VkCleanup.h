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

#ifndef KDO_VKCLEANUP_H
# define KDO_VKCLEANUP_H

#define KDO_DESTROY(vkDestroy, device, object)  if (object) {\
										vkDestroy(device, object, NULL);\
										object = NULL; }

#define KDO_FREE(object)				free(object);\
										object = NULL;
										
# include "kdo_Vulkan.h"

void    kdo_freePhysicalDevice(Kdo_Vulkan *vk);
void    kdo_swapChainCleanup(Kdo_Vulkan *vk);
void    kdo_freeBuffer(Kdo_Vulkan *vk, Kdo_VkBuffer *buffer);
void    kdo_freeImage(Kdo_Vulkan *vk, Kdo_VkImageBuffer *image);
void    kdo_cleanup(Kdo_Vulkan *vk, char *msg, int returnCode);

#endif
