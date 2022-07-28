#version 330 core

uniform mat4 modelMatrix;
uniform mat4 lightMatrix;

in vec3 position;

void main(void)
{
	gl_Position = ((lightMatrix * modelMatrix) * vec4(position, 1.0f));
}