#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D bumpTex;
uniform sampler2D shadowTex;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;
	vec3 tangent;
	vec3 binormal;
	vec4 shadowProj;
}IN;

out vec4 fragColour[3];

float ShadowShader();

void main(void)
{
	mat3 TBN		= mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
	
	vec3 normal		= texture2D(bumpTex, IN.texCoord).rgb * 2.0 - 1.0;
	normal			= normalize(TBN * normalize(normal));
	
	fragColour[0]	= texture2D(diffuseTex, IN.texCoord) * ShadowShader();
	fragColour[1]	= vec4(normal.xyz * 0.5 + 0.5, 1.0);

}

float ShadowShader()
{
	float	shadow		= 1.0;
	vec3	shadowNDC	= IN.shadowProj.xyz / IN.shadowProj.w;
	if(abs(shadowNDC.x) < 1.0f && abs(shadowNDC.y) < 1.0f && abs(shadowNDC.z) < 1.0f)
	{
		vec3 biasCoord	= shadowNDC * 0.5f + 0.5f;
		float shadowZ	= texture(shadowTex, biasCoord.xy).x;
		if(shadowZ < biasCoord.z)
		{
			shadow = 0.0f;
		}
	}

	return shadow;
}