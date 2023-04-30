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

#ifndef KDO_VKSWAPCHAIN_H
# define KDO_VKSWAPCHAIN_H

# include "kdo_Vulkan.h"
# include "kdo_VkPhysicalDevice.h"
# include "kdo_VkRenderPass.h"
# include "kdo_VkFramebuffers.h"
# include "kdo_VkSyncronisation.h"

void	kdo_initSwapChain(Kdo_Vulkan *vk);
void	kdo_recreateSwapChain(Kdo_Vulkan *vk);

#endif
