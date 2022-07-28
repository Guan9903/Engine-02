#version 330 core

struct DirLight
{
	vec3	direction;
};

struct PointLight
{
	float	radius;
};

uniform sampler2D	depthTex;
uniform sampler2D	normTex;
uniform vec2		pixelSize;
uniform vec3		cameraPos;
uniform mat4		inverseProjView;
uniform vec4		lightColour;
uniform vec3		lightPos;
//uniform float		lightRadius;
uniform float       shininess;  //the specular reflection value

uniform DirLight	dirLight;
uniform PointLight	pointLight;

out vec4 diffuseOutput;
out vec4 specularOutput;

vec3 DirLightCal(DirLight light, vec3 norm, vec3 viewDir, vec3 halfDir);

void main(void)
{
	vec2 texCoord	= vec2(gl_FragCoord.xy * pixelSize);
	float depth		= texture(depthTex, texCoord.xy).r;
	vec3 ndcPos		= vec3(texCoord, depth) * 2.0 - 1.0;
	vec4 invClipPos = inverseProjView * vec4(ndcPos, 1.0);
	vec3 worldPos	= invClipPos.xyz / invClipPos.w;
	
	//float dist		= length(lightPos - worldPos);
	//float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);
	//float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);
	
	//if(atten == 0.0)
	//	discard;
	
	vec3 normal		= normalize(texture(normTex, texCoord.xy).xyz * 2.0 - 1.0);
	vec3 incident	= normalize(lightPos - worldPos);
	vec3 viewDir	= normalize(cameraPos - worldPos);
	vec3 halfDir	= normalize(incident + viewDir);
	
	vec3 lightDir		= normalize(-dirLight.direction);
	float lambert		= clamp(dot(lightDir, normal), 0.0, 1.0);
	float rFactor		= clamp(dot(halfDir, normal), 0.0, 1.0);
	float specFactor	= clamp(dot(halfDir, normal), 0.0, 1.0);
	specFactor			= pow(specFactor, 60.0);

	//vec3 output = DirLightCal(dirLight, normal, viewDir, halfDir);
	//vec3 attenuated = lightColour.xyz * atten;
	//diffuseOutput = vec4(attenuated * lambert, 1.0);
	//specularOutput = vec4(attenuated * specFactor * 0.33, 1.0);
	diffuseOutput = vec4(lightColour.xyz * lambert, 1.0);
	specularOutput = vec4(lightColour.xyz * specFactor * 0.33, 1.0);
}

vec3 DirLightCal(DirLight light, vec3 norm, vec3 viewDir, vec3 halfDir)
{
	vec3 lightDir		= normalize(-light.direction);

	float lambert		= clamp(dot(lightDir, norm), 0.0, 1.0);
	float rFactor		= clamp(dot(halfDir, norm), 0.0, 1.0);
	float specFactor	= clamp(dot(halfDir, norm), 0.0, 1.0);
	specFactor			= pow(specFactor, 60.0);

	vec3 diffuseColour	= lightColour.rgb;
	return diffuseColour;
}