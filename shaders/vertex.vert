#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inTexCoord;

layout(location = 0) out vec3 fragColor;

layout( push_constant ) uniform constants
{
	mat4 mvp;
} push;

void main()
{
    gl_Position = push.mvp * vec4(inPos, 1.0);
    fragColor = inColor;
}
