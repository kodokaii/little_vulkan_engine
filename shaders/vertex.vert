#version 450
#extension GL_ARB_separate_shader_objects : enable

struct Object
{
    mat4	modelMat;
    mat4	normalMat;
	uint	vertexCount;
	uint	instanceCount;
	uint	firstVertex;
	uint	firstInstance;
};

layout(std430, binding = 0) readonly buffer ObjectBuffer
{
    Object	objectArray[];
} objectBuffer;

layout(std430, binding = 1) readonly buffer Vector3Buffer
{
	vec3	vec3Array[];	
} vector3Buffer;

layout(std430, binding = 2) readonly buffer Vector2Buffer
{
	vec2	vec2Array[];	
} vector2Buffer;

layout(push_constant) uniform Push
{
	mat4	camera;
	vec4	cameraPos;
} push;


layout(location = 0) in uint	inPosIndex;
layout(location = 1) in uint	inTangentIndex;
layout(location = 2) in uint	inBitangentIndex;
layout(location = 3) in uint	inNormalIndex;
layout(location = 4) in uint	inUVIndex;
layout(location = 5) in uint	inMtlIndex;

layout(location = 0) out vec3	outPos;
layout(location = 1) out vec3	outTangent;
layout(location = 2) out vec3	outBitangent;
layout(location = 3) out vec3	outNormal;
layout(location = 4) out vec2	outUV;
layout(location = 5) out uint	outMtlIndex;

void main()
{ 
	mat4	inModelMat	= objectBuffer.objectArray[gl_InstanceIndex].modelMat;
	mat4	inNormalMat	= objectBuffer.objectArray[gl_InstanceIndex].normalMat;
	vec3	inPos		= vector3Buffer.vec3Array[inPosIndex];
	vec3	inTangent	= vector3Buffer.vec3Array[inTangentIndex];
	vec3	inBitangent	= vector3Buffer.vec3Array[inBitangentIndex];
	vec3	inNormal	= vector3Buffer.vec3Array[inNormalIndex];
	vec2	inUV		= vector2Buffer.vec2Array[inUVIndex];
	
    gl_Position			= push.camera * inModelMat * vec4(inPos, 1);
	outPos				= mat3(inModelMat) * inPos;
	outTangent			= normalize(mat3(inNormalMat) * inTangent);
	outBitangent		= normalize(mat3(inNormalMat) * inBitangent);
	outNormal			= normalize(mat3(inNormalMat) * inNormal);
	outUV				= inUV;
	outMtlIndex			= inMtlIndex;
}
