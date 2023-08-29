#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_LIGHT           8
#define MAX_TEXTURE			128
#define MAX_MATERIAL        128

struct Light
{
    vec4    pos_intensity;
    vec4    color_stop;
};

struct Material
{
    vec4        Ka;
    vec4        Kd;
    vec4        Ks;
    vec4        Ke;
    vec4        Kt;
    vec4        Tf;
    float       Ns;
    float       Ni;
    float       d;
    int         illum;
    uint        map_Kd;
    uint        map_Ns;
    uint        map_Bump;
};

layout(location = 0) in vec3		inPos;
layout(location = 1) in vec3		inNormal;
layout(location = 2) in vec2		inTexCoord;
layout(location = 3) flat in uint	inMaterialIndex;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform sampler samp;
layout(binding = 3) uniform texture2D textures[MAX_TEXTURE];
layout(binding = 4) uniform LightBuffer
{
    Light       light[MAX_LIGHT];
}	lightBuffer;

layout(binding = 5) uniform MaterialBuffer
{
    Material    material[MAX_MATERIAL];
} materialBuffer;


layout(push_constant) uniform Push
{
    mat4    camera;
    vec4    cameraPos;
} push;

void main() {

	Material material;

	vec3	ambientLight;

	vec3	lightDir;
	vec3	viewDir;
	vec3	reflectDir;

	vec3	ambient;
	vec3	diffuse;
	vec3	specular;

	material		= materialBuffer.material[inMaterialIndex];

	ambientLight	= vec3(0.01, 0.01, 0.01);

	lightDir		= normalize(vec3(lightBuffer.light[0].pos_intensity) - inPos);
	viewDir			= normalize(vec3(push.cameraPos) - inPos);
	reflectDir		= reflect(-lightDir, inNormal);

	ambient			= vec3(material.Ka) * ambientLight;
	diffuse			= vec3(material.Kd) * vec3(lightBuffer.light[0].color_stop) * max(dot(inNormal, lightDir), 0.0);
	specular		= vec3(material.Ks) * vec3(lightBuffer.light[0].color_stop) * pow(max(dot(viewDir, reflectDir), 0.0), material.Ns);

	outColor = vec4(ambient + diffuse + specular, 1);
}
