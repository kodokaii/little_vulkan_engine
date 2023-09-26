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

#ifndef KDO_VKBST_H
# define KDO_VKBST_H

# include <stdlib.h>
# include <string.h>
# include <stdint.h>

typedef struct Kdo_BST
{
	struct Kdo_BST	*left;
	struct Kdo_BST	*right;
	uint32_t		index;
}	Kdo_BST;

void    *kdo_BSTaddData(Kdo_BST **root, void *buffer, size_t sizeData, uint32_t *index, void *data);
char    *kdo_BSTaddStr(Kdo_BST **root, char **strBuffer, uint32_t *index, char *str);

#endif
