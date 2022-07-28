#version 330 core

uniform samplerCube cubeTex;

uniform vec3 cameraPos;

in Vertex
{
	vec3 viewDir;
}IN;

out vec4 fragColour;

void main(void)
{
	fragColour = texture(cubeTex, normalize(IN.viewDir));
}