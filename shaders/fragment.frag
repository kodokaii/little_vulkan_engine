#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler samp;
layout(binding = 1) uniform texture2D textures[2];

layout( push_constant ) uniform constants
{
	layout(offset = 128) uint objIndex;
} push;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(inColor * texture(sampler2D(textures[push.objIndex], samp), inTexCoord).rgb, 1.0);
}
