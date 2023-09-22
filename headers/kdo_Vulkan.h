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
# include "cglm/cglm.h"

# define GLFW_INCLUDE_VULKAN
# include <GLFW/glfw3.h>

# include <stdlib.h>
# include <stdio.h>
# include <string.h>

# define SZ_SHADER				2

# define QUEUES_COUNT			3
# define PRESENT_QUEUE			0
# define GRAPHIC_QUEUE			1
# define TRANSFER_QUEUE			2

#define	VERTEX_BUFFER_SIZE		128
#define	VECTOR3_BUFFER_SIZE		128
#define	VECTOR2_BUFFER_SIZE		128
#define	MATERIAL_BUFFER_SIZE	128
#define	LIGHT_BUFFER_SIZE		128

#define MAX_TEXTURES			32
# define TEXTURE_BUFFER_SIZE	100000000
# define DEFAULT_TEXTURE		"textures/default.png"

# define ERRLOC					"CPU Memory allocation error"
# define FOR_NO_ERROR			-1

typedef enum Kdo_VkWait
{
	WAIT_DEVICE	= 0x00000001
}	Kdo_VkWait;

typedef enum Kdo_VkStatus
{
	INVISIBLE	= 0x00000001
}	Kdo_VkStatus;

typedef struct Kdo_ShMaterial
{
	vec3		ambient;
    uint32_t	ambientMap;
    vec3		diffuse;
    uint32_t	diffuseMap;
    vec3		specular;
    uint32_t	specularMap;
    vec3		emissive;
    uint32_t	emissiveMap;
    vec3		transmittance;
    uint32_t	transmittanceMap;
    vec3		transmissionFilter;
    uint32_t	transmissionFilterMap;
	float		shininess;
    uint32_t	shininessMap;
    float		refractionIndex;
    uint32_t	refractionMap;
    float		disolve;
    uint32_t	disolveMap;
    int			illum;
    uint32_t	bumpMap;
}	Kdo_ShMaterial;

typedef struct Kdo_ShObject
{
	mat4							modelMat;
    mat4							normalMat;
	VkDrawIndirectCommand			drawCommand;
}	Kdo_ShObject;

typedef struct Kdo_ShLight
{
	vec4	pos_intensity;
	vec4	color_stop;
}	Kdo_ShLight;

typedef struct Kdo_VkPush
{
	mat4	camera;
	vec4	cameraPos;
}	Kdo_VkPush;

typedef struct Kdo_VkVertex
{
	uint32_t	posIndex;
	uint32_t	tangentIndex;
	uint32_t	bitangentIndex;
	uint32_t	normalIndex;
	uint32_t	uvIndex;
	uint32_t	mtlIndex;
}	Kdo_VkVertex;

typedef struct Kdo_Shader
{
	char			*code;
	long			codeSize;
	VkShaderModule  module;
}	Kdo_Shader;

typedef struct Kdo_VkDepthBuffer
{
	VkImage			image;
	VkDeviceMemory	memory;
	VkImageView		view;
	VkFormat		format;
}	Kdo_VkDepthBuffer;

typedef struct Kdo_VkQueue
{
	VkQueue		path;
	uint32_t	index;
	uint32_t	familyIndex;
}	Kdo_VkQueue;

typedef struct Kdo_SynImage
{
	struct Kdo_SynImage	**path;
	VkFence				*renderFinishedFence;
}	Kdo_SynImage;

typedef struct Kdo_VkQueueInfo
{ uint32_t		count; float			priority; VkQueueFlags	requiredFlags; VkQueueFlags	noRequiredFlags;
	VkQueueFlags	preferredFlags;
	VkQueueFlags	noPreferredFlags;
	VkBool32		presentSupport;
}	Kdo_VkQueueInfo;

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
	Kdo_VkSwapChainProperties			swapChainProperties; Kdo_VkQueueProperties				queueProperties; VkPhysicalDeviceMemoryProperties	memProperties; }	Kdo_VkPhysicalDevice;

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
	Kdo_Shader				vertexShader;
	Kdo_Shader				fragmentShader;
	VkDescriptorSetLayout   descriptorLayout;
	VkPipelineLayout		layout;
}	Kdo_VkGraphicsPipeline;

typedef struct Kdo_VkFramebuffer
{
	VkFramebuffer		*path;
	Kdo_VkDepthBuffer	depth;
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

typedef struct Kdo_VkBufferProperties
{
	VkBufferUsageFlags		usage;
	VkMemoryPropertyFlags	memoryFlags;
	Kdo_VkWait				waitFlags;
}	Kdo_VkBufferProperties;

typedef struct Kdo_VkBuffer
{
	Kdo_VkBufferProperties	properties;
	VkBuffer				buffer;
	VkDeviceMemory			memory;
	VkDeviceSize			sizeUsed;
	VkDeviceSize			sizeFree;
}	Kdo_VkBuffer;

typedef struct Kdo_VkImageProperties
{
	VkImageLayout			layout;
	VkMemoryPropertyFlags	memoryFlags;
	uint32_t				memoryFilter;
	Kdo_VkWait				waitFlags;
}	Kdo_VkImageProperties;

typedef struct Kdo_VkImage
{
	VkExtent3D				extent;
	VkDeviceSize			size;
	VkImage					image;
	VkImageView				view;
}	Kdo_VkImage;

typedef	struct Kdo_VkImageBuffer
{
	Kdo_VkImageProperties	properties;
	VkDeviceMemory			memory;
	VkDeviceSize			sizeUsed;
	VkDeviceSize			sizeFree;
	uint32_t				imageCount;
	Kdo_VkImage				*image;
}	Kdo_VkImageBuffer;

typedef	struct Kdo_VkBufferCoreElement
{
	Kdo_VkBuffer	path;
	uint32_t		count;
	uint32_t		*sort;
}	Kdo_VkBufferCoreElement;

typedef	struct Kdo_VkImageCoreElement
{
	Kdo_VkImageBuffer	path;
	char				**name;
	uint32_t			*sort;
}	Kdo_VkImageCoreElement;

typedef struct Kdo_VkBufferCore
{
	Kdo_VkBufferCoreElement		vertex;
	Kdo_VkBufferCoreElement		vector3;
	Kdo_VkBufferCoreElement		vector2;
	Kdo_VkBufferCoreElement		material;
	Kdo_VkBufferCoreElement		light;
	Kdo_VkImageCoreElement		texture;
	Kdo_VkBufferCoreElement		object;
}	Kdo_VkBufferCore;

typedef struct Kdo_VkBufferCoreCount
{
	uint32_t	vertex;
	uint32_t	vector3;
	uint32_t	vector2;
	uint32_t	material;
	uint32_t	light;
	uint32_t	object;
}	Kdo_VkBufferCoreCount;

typedef struct Kdo_VkSampler
{
	VkSampler	basic;
}	Kdo_VkSampler;

typedef struct Kdo_VkCore
{
	VkCommandPool			transferPool;
	VkDescriptorPool		descriptorPool;
	VkDescriptorSet			descriptorSet;
	Kdo_VkBufferCore		buffer;
	Kdo_VkSampler			sampler;
}	Kdo_VkCore;

typedef struct Kdo_VkRenderPool
{
	VkCommandPool	path;
	VkCommandBuffer	main;
}	Kdo_VkRenderPool;

typedef struct Kdo_VkDisplay
{
	Kdo_VkRenderPool	*renderPool;
	uint32_t			currentImage;
	uint32_t			currentFrame;
	uint32_t			maxParallelFrame;
	uint32_t			windowResized;
	Kdo_SynImage		*frameToImage;
	Kdo_SynImage		**imageToFrame;
}	Kdo_VkDisplay;

typedef struct Kdo_VkCamera
{
	mat4	path;
	vec3	pos;
	double	moveTime;
	double  xMouse;
	double	yMouse;
	float	yaw;
	float	pitch;
} Kdo_VkCamera;

typedef struct Kdo_VkCompute
{
	int		fps;
	double	startTime;
	double	currentTime;
	double	secondTime;
}	Kdo_VkCompute;

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
	Kdo_VkCore				core;
	Kdo_VkDisplay			display;
	Kdo_VkCamera			camera;
	Kdo_VkCompute			compute;
}	Kdo_Vulkan;

# include "kdo_VkCleanup.h"
# include "kdo_VkFunction.h"

#endif
