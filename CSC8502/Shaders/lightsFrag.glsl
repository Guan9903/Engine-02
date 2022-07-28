#version 330 core

struct DirLight
{
	vec3	direction;
	vec3	ambient;
    vec3	diffuse;
    vec3	specular;
};

struct PointLight
{
	vec3	position;
	float	radius;
	vec3	ambient;
    vec3	diffuse;
    vec3	specular;
};

struct SpotLight
{
	vec3	position;
	vec3	direction;
};

in Vertex
{
    vec2 texCoord;
	vec3 worldPos;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec4 shadowProj;
}IN;

uniform sampler2D   diffuseTex;
uniform sampler2D   bumpTex;
uniform sampler2D	shadowTex;
uniform float       shininess;  //the specular reflection value

uniform vec3        cameraPos;
uniform vec4        lightColour;
uniform vec3        lightPos;

uniform DirLight	dirLight;
uniform SpotLight	spotLight;

out vec4 fragColour;

vec3 DirLightShader(DirLight light, vec3 viewDir, vec3 halfDir, mat3 tbn);
vec3 PointLightShader(PointLight light);

float ShadowShader();

void main(void)
{
	vec3 incident   = normalize(lightPos - IN.worldPos);
	//float is_incone = 1.0f;
	vec3 viewDir    = normalize(cameraPos - IN.worldPos);
	//if(abs(dot(normalize(spotLight.position - IN.worldPos),dirLight.direction))>0.5f){is_incone=0.0f;}
	vec3 halfDir    = normalize(incident + lightPos);
    mat3 TBN        = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

    vec3 output		= DirLightShader(dirLight, viewDir, halfDir, TBN);
	fragColour		= vec4(output, 1.0);
	//fragColour     *= is_incone;
	fragColour.rgb *= ShadowShader();
}

vec3 DirLightShader(DirLight light, vec3 viewDir, vec3 halfDir, mat3 tbn)
{
	vec3 lightDir		= normalize(-light.direction);

    //diffuse
    vec4 diffuse		= texture(diffuseTex, IN.texCoord);
	vec3 bumpNormal		= texture(bumpTex, IN.texCoord).rgb;
	bumpNormal			= normalize(tbn * normalize(bumpNormal * 2.0 - 1.0));	
	
	vec3 norm			= normalize(IN.normal);
	float lambert		= max(dot(lightDir, norm), 0.0f);

    //specular
    float specFactor	= clamp(dot(halfDir, IN.normal), 0.0, 1.0);
	specFactor			= pow(specFactor, shininess);

	vec3 reflectDir		= reflect(-lightDir, norm);
	float spec			= pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    //combine results
    vec3 diffuseColour	= diffuse.rgb * lightColour.rgb * lambert;
    vec3 specularColour = lightColour.rgb * spec;
	vec3 ambientColour	= diffuseColour * 0.2f;
    return (diffuseColour  + ambientColour + specularColour);
}

float ShadowShader()
{
	float shadow = 1.0;
	vec3 shadowNDC = IN.shadowProj.xyz / IN.shadowProj.w;
	if(abs(shadowNDC.x) < 1.0f && abs(shadowNDC.y) < 1.0f && abs(shadowNDC.z) < 1.0f)
	{
		vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
		float shadowZ = texture(shadowTex, biasCoord.xy).x;
		if(shadowZ < biasCoord.z - 0.01)
		{
			shadow = 0.5f;
		}
	}

//	vec3 biasCoord = shadowNDC * 0.5f + 0.5f;
//	float shadowZ = texture(shadowTex, biasCoord.xy).r;
//	shadow = biasCoord.z > shadowZ ? 1.0 : 0.0;
	return shadow;
}