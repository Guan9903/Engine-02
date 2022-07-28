#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 lightDir;
uniform vec3 lightPos;
uniform vec4 lightColour;

in vec3 position;

void main(void)
{
	vec3 direction = lightDir;
	vec3 worldPos = position + lightPos;
	gl_Position = (projMatrix * viewMatrix ) * vec4(worldPos, 1.0);
}