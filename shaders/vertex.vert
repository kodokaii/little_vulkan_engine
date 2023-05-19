#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec2 outTexCoord;

layout( push_constant ) uniform constants
{
	mat4 mvp;
	mat3 normalMat;
} push;

void main()
{
    gl_Position		= push.mvp * vec4(inPos, 1.0);
	outNormal		= push.normalMat * inNormal;
    outColor		= inColor;
	outTexCoord		= inTexCoord;
}
