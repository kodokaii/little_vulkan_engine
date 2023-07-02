#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_OBJECT			32
#define MAX_TEXTURES		64
#define MAX_MATERIAL_MAP	256

struct ObjectMap
{
    mat4 modelMat;
    mat4 normalMat;
	uint materialOffset;
	uint pad0;
	uint pad1;
	uint pad2;
};

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in uint inRelMaterialIndex;

layout(location = 0) out vec3	outPos;
layout(location = 1) out vec3	outNormal;
layout(location = 2) out vec2	outTexCoord;
layout(location = 3) out uint	outMaterialIndex;

layout(binding = 0) uniform ObjectBuffer
{
    ObjectMap	object[MAX_OBJECT];
} objectBuffer;

layout(binding = 1) uniform MaterialMapBuffer
{
	uint		absMaterialIndex[MAX_MATERIAL_MAP];
} materialMapBuffer;

layout(push_constant) uniform Push
{
	mat4	camera;
	vec4	cameraPos;
} push;

void main()
{ 
    gl_Position			= push.camera * objectBuffer.object[gl_InstanceIndex].modelMat * vec4(inPos, 1.0);
	outPos				= vec3(objectBuffer.object[gl_InstanceIndex].modelMat * vec4(inPos, 1.0));
	outNormal			= normalize(mat3(objectBuffer.object[gl_InstanceIndex].normalMat) * inNormal);
	outTexCoord			= inTexCoord;

	outMaterialIndex	= inRelMaterialIndex;
}
