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

#ifndef KDO_VULKAN_H
# define KDO_VULKAN_H

# define CGLM_FORCE_LEFT_HANDED
# define CGLM_FORCE_DEPTH_ZERO_TO_ONE
# define STB_IMAGE_IMPLEMENTATION
# include "cglm/cglm.h"
//# include "stb/stb_image.h"

# define GLFW_INCLUDE_VULKAN
# include <GLFW/glfw3.h>

# include <stdlib.h>
# include <stdio.h>
# include <string.h>

# define QUEUES_COUNT	3
#define PRESENT_QUEUE   0
#define GRAPHIC_QUEUE   1
#define TRANSFER_QUEUE  2

# define ERRLOC			"CPU Memory allocation error"
# define FOR_NO_ERROR	0

typedef enum Kdo_ObjectStatus
{
	Invisible	= 0x00000001
}	Kdo_ObjectStatus;

typedef struct Kdo_VkQueueInfo
{
	uint32_t		count;
	float			priority;
	VkQueueFlags	requiredFlags;
	VkQueueFlags	noRequiredFlags;
	VkQueueFlags	preferredFlags;
	VkQueueFlags	noPreferredFlags;
	VkBool32		presentSupport;
}	Kdo_VkQueueInfo;

typedef struct Kdo_VkBufferCreateInfo
{
	VkBufferCreateInfo		path;
	VkMemoryPropertyFlags	memoryFlags;
	VkBuffer				*buffer;
	VkDeviceMemory			*memory;
}	Kdo_BufferCreateInfo;

typedef struct Kdo_ImageCreateInfo
{
	VkImageCreateInfo		path;
	VkMemoryPropertyFlags	memoryFlags;
	VkImage					*image;
	VkDeviceMemory			*memory;
} Kdo_ImageCreateInfo;


typedef struct Kdo_VkQueue
{
	VkQueue		path;
	uint32_t	index;
	uint32_t	familyIndex;
}	Kdo_VkQueue;

typedef struct Kdo_Vertex
{
    vec3	pos;
    vec3	color;
	vec3	tex;
	vec3	normal;
}	Kdo_Vertex;

typedef struct Kdo_Shader
{
	char	*code;
	long	codeSize;
}	Kdo_Shader;

typedef struct Kdo_SynImage
{
	struct Kdo_SynImage	**path;
	VkFence				*renderFinishedFence;
}	Kdo_SynImage;

typedef	struct Kdo_VkInitInfo
{
	const char					*applicationName;
	const char					*engineName;
	uint32_t					instanceExtensionsCount;
	const char					**instanceExtensions;
	uint32_t					validationLayersCount;
	const char					**validationLayers;
	uint32_t					deviceExtensionsCount;
	const char					**deviceExtensions;
	int							windowWidth;
	int							windowHeight;
	Kdo_VkQueueInfo				queuesInfo[QUEUES_COUNT];
	VkPresentModeKHR			presentMode;
	vec3						startPos;
	float						startYaw;
	float						startPitch;
	
}	Kdo_VkInitInfo;

typedef struct Kdo_VkWindow
{
	GLFWwindow	*path;
}	Kdo_VkWindow;

typedef struct Kdo_VkInstance
{
	VkInstance	path;
}	Kdo_VkInstance;

typedef struct Kdo_VkSurface
{
	VkSurfaceKHR	path;
}	Kdo_VkSurface;

typedef struct Kdo_VkSwapChainProperties
{
	VkSurfaceCapabilitiesKHR	capabilities;
	uint32_t					formatsCount;
    VkSurfaceFormatKHR			*formats;
	uint32_t					presentModesCount;
    VkPresentModeKHR			*presentModes;
}	Kdo_VkSwapChainProperties;

typedef struct Kdo_VkQueueProperties
{
	VkQueueFamilyProperties	*path;
	VkBool32				*presentSupport;
	uint32_t				famillesCount;
}	Kdo_VkQueueProperties;

typedef struct Kdo_VkPhysicalDevice
{
	VkPhysicalDevice					path;
	VkPhysicalDeviceProperties			properties;
	VkPhysicalDeviceFeatures			features;
	Kdo_VkSwapChainProperties			swapChainProperties;
	Kdo_VkQueueProperties				queueProperties;
	VkPhysicalDeviceMemoryProperties	memProperties;
}	Kdo_VkPhysicalDevice;

typedef struct Kdo_VkDevice
{
	VkDevice	path;
	Kdo_VkQueue	queues[QUEUES_COUNT];
}	Kdo_VkDevice;

typedef struct Kdo_VkSwapChain
{
	VkSwapchainKHR		path;
	VkSwapchainKHR		old;
	uint32_t			imagesCount;
	VkImage				*images;
	VkImageView			*views;
	VkExtent2D			imagesExtent;
	VkSurfaceFormatKHR	imagesFormat;
}	Kdo_VkSwapChain;

typedef struct Kdo_VkRenderPass
{
	VkRenderPass	path;
}	Kdo_VkRenderPass;

typedef struct Kdo_VkGraphicsPipeline
{
	VkPipeline				path;
	Kdo_Shader				VertexShader;
	Kdo_Shader				FragmentShader;
	VkPipelineLayout		layout;
}	Kdo_VkGraphicsPipeline;

typedef struct Kdo_VkFramebuffer
{
	VkFramebuffer	*path;
}	Kdo_VkFramebuffer;

typedef struct Kdo_VkSemaphore
{
	VkSemaphore	*imageAvailable;
	VkSemaphore *renderFinished;
}	Kdo_VkSemaphore;

typedef struct Kdo_VkFence
{
	VkFence	*renderFinished;
}	Kdo_VkFence;

typedef struct Kdo_VkObject
{
	char				*name;
	void				*data;
	uint32_t			count;
	mat4				*model;
	Kdo_ObjectStatus	status;
	VkDeviceSize		vertexCount;
	VkDeviceSize		indexCount;
	VkDeviceSize		textureSize;
	VkDeviceSize		vertexOffset;
	VkDeviceSize		indexOffset;
	VkDeviceSize		textureOffset;
	VkDeviceSize		descriptorOffset;
	struct Kdo_VkObject	*next;
}	Kdo_VkObject;

typedef struct Kdo_VkRender
{
	VkDeviceSize		vertexCount;
	VkDeviceSize		indexCount;
	VkDeviceSize		textureSize;
	VkBuffer			stageVertexBuffer;
	VkBuffer			stageTextureBuffer;
	VkBuffer			stageDescriptorBuffer;
	VkBuffer			vertexBuffer;
	VkBuffer			textureBuffer;
	VkBuffer			descriptorBuffer;
	VkDeviceMemory		stageVertexMemory;
	VkDeviceMemory		stageTextureMemory;
	VkDeviceMemory		stageDescriptorMemory;
	VkDeviceMemory		vertexMemory;
	VkDeviceMemory		textureMemory;
	VkDeviceMemory		descriptorMemory;
	VkCommandPool		transfertPool;
	VkCommandPool		renderPool;
	VkDescriptorPool	descriptorPool;
	VkDescriptorSet		descriptorSet;
	VkCommandBuffer		renderbuffer;
	Kdo_VkObject		*objects;
	uint32_t			objectsCount;
}	Kdo_VkRender;

typedef struct Kdo_VkDisplay
{
	uint32_t		currentImage;
	uint32_t		currentFrame;
	uint32_t		maxParallelFrame;
	uint32_t		windowResized;
	Kdo_SynImage	*imageLinks;
	Kdo_SynImage	**fenceLinks;
}	Kdo_VkDisplay;

typedef struct Kdo_VkCamera
{
	double	moveTime;
	double  xMouse;
	double	yMouse;
	float	yaw;
	float	pitch;
	vec3	pos;
} Kdo_VkCamera;


typedef struct Kdo_Vulkan
{
	Kdo_VkInitInfo			info;
	Kdo_VkWindow			window;
	Kdo_VkInstance			instance;
	Kdo_VkSurface			surface;
	Kdo_VkPhysicalDevice	physicalDevice;
	Kdo_VkDevice			device;
	Kdo_VkSwapChain			swapChain;
	Kdo_VkRenderPass		renderPass;	
	Kdo_VkGraphicsPipeline	graphicsPipeline;
	Kdo_VkFramebuffer		framebuffer;
	Kdo_VkSemaphore			semaphore;
	Kdo_VkFence				fence;
	Kdo_VkRender			render;
	Kdo_VkDisplay			display;
	Kdo_VkCamera			camera;
}	Kdo_Vulkan;

# include "kdo_VkCleanup.h"
# include "kdo_VkFunction.h"

#endif
