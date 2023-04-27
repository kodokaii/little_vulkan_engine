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
void    kdo_swapchainCleanup(Kdo_Vulkan *vk);
void    kdo_cleanup(Kdo_Vulkan *vk, char *msg, int returnCode);

#endif
