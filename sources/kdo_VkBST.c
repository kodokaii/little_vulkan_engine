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

#include "kdo_VkBST.h"

Kdo_BST	*kdo_BSTCreateNode(uint32_t	index)
{
	Kdo_BST	*node;

	node = malloc(sizeof(Kdo_BST));
	if (node)
		node->index	= index;
	return (node);
}

void	*kdo_BSTaddData(Kdo_BST **root, void *buffer, size_t sizeData, uint32_t *index, void *data)
{
	int	cmp;

	if (*root)
	{
		cmp = memcmp(buffer + (*root)->index * sizeData, data, sizeData);
		if (0 < cmp)
			return (kdo_BSTaddData(&(*root)->left, buffer, sizeData, index, data));
		else if (cmp < 0)
			return (kdo_BSTaddData(&(*root)->right, buffer, sizeData, index, data));
		else
			*index = (*root)->index;
	}
	else
	{
		if (!(*root = kdo_BSTCreateNode(*index)))
			return (NULL);
		memcpy(buffer + *index * sizeData, data, sizeData);
	}

	return (buffer + *index * sizeData);
}

char	*kdo_BSTaddStr(Kdo_BST **root, char **strBuffer, uint32_t *index, char *str)
{
	int	cmp;

	if (*root)
	{
		cmp = strcmp(strBuffer[(*root)->index], str);
		if (0 < cmp)
			return (kdo_BSTaddStr(&(*root)->left, strBuffer, index, str));
		else if (cmp < 0)
			return (kdo_BSTaddStr(&(*root)->right, strBuffer, index, str));
		else
			*index = (*root)->index;
	}
	else
	{
		if (!(*root = kdo_BSTCreateNode(*index)))
			return (NULL);
		strBuffer[*index] = str;
	}

	return (strBuffer[*index]);
}
