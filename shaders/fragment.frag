#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_TEXTURE	32

struct Material
{
    vec3	ambient;
    uint	ambientMap;
    vec3	diffuse;
    uint	diffuseMap;
    vec3	specular;
    uint	specularMap;
    vec3	emissive;
    uint	emissiveMap;
    vec3	transmittance;
    uint	transmittanceMap;
    vec3	transmissionFilter;
    uint	transmissionFilterMap;
    float	shininess;
    uint	shininessMap;
    float	refractionIndex;
    uint	refractionMap;
    float	disolve;
    uint	disolveMap;
    int		illum;
    uint	bumpMap;
};

struct Light
{
    vec4    pos_intensity;
    vec4    color_stop;
};

layout(binding = 3) uniform sampler defaultSampler;

layout(binding = 4) uniform texture2D tex[MAX_TEXTURE];

layout(std430, binding = 5) readonly buffer MaterialBuffer
{
    Material	materialArray[];
}	materialBuffer;

layout(std430, binding = 6) readonly buffer LightBuffer
{
    Light       lightArray[];
}	lightBuffer;

layout(push_constant) uniform Push
{
    mat4    camera;
    vec4    cameraPos;
} push;

layout(location = 0) in vec3		inPos;
layout(location = 1) in vec3		inTangent;
layout(location = 2) in vec3		inBitangent;
layout(location = 3) in vec3		inNormal;
layout(location = 4) in vec2		inUV;
layout(location = 5) flat in uint	inMtlIndex;

layout(location = 0) out vec4 outColor;

void main() {

	Material mtl			= materialBuffer.materialArray[inMtlIndex];

	vec3	ambientLight	= vec3(0.01, 0.01, 0.01);
	vec3	lightPos		= vec3(lightBuffer.lightArray[0].pos_intensity);
	vec3	lightColor		= vec3(lightBuffer.lightArray[0].color_stop);

	vec3	lightDir		= normalize(lightPos - inPos);
	vec3	viewDir			= normalize(vec3(push.cameraPos) - inPos);
	vec3	reflectDir		= reflect(-lightDir, inNormal);

	vec3	diffuseMap		= vec3(texture(sampler2D(tex[mtl.diffuseMap], defaultSampler), inUV));
	vec3	specularMap		= mtl.specularMap != 0	? vec3(texture(sampler2D(tex[mtl.specularMap], defaultSampler), inUV))	: vec3(1, 1, 1);
	vec3	bumpMap			= mtl.bumpMap != 0		? vec3(texture(sampler2D(tex[mtl.bumpMap], defaultSampler), inUV))		: vec3(0, 0, 1);

	vec3	ambient			= mtl.ambient * ambientLight;
	vec3	diffuse			= mtl.diffuse * diffuseMap * lightColor * max(dot(inNormal, lightDir), 0.0);
	vec3	specular		= mtl.specular * specularMap * lightColor * pow(max(dot(viewDir, reflectDir), 0.0), mtl.shininess);

	outColor = vec4(ambient + diffuse + specular, 1);
}
