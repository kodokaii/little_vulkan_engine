#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outTexCoord;

layout( push_constant ) uniform constants
{
	mat4 mvp;
	mat4 normalMat;
} push;

void main()
{ 
    gl_Position		= push.mvp * vec4(inPos, 1.0);
    outColor		= inColor * max(dot(normalize(mat3(push.normalMat) * inNormal), normalize(vec3(1.0, -3.0, -1.0))), 0.0);
	outTexCoord		= inTexCoord;
}
