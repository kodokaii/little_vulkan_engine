#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = vec4(inColor * texture(texSampler, inTexCoord).rgb * max(dot(inNormal, normalize(vec3(1.0, 1.0, 1.0))), 0.01), 1.0);
}
