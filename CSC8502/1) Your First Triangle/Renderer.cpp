#include "Renderer.h"

#pragma region task 1

Renderer::Renderer(Window& parent) : OGLRenderer(parent)
{
	triangle = Mesh::GenerateTriangle();

	basicShader = new Shader("basicVertex.glsl", "colourFragment.glsl");

	if (!basicShader->LoadSuccess())
		return;

	init = true;
}

Renderer::~Renderer(void)
{
	delete triangle;
	delete basicShader;
}

void Renderer::RenderScene()
{
	glClearColor(0.2f, 0.2f, 0.2f, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);

	BindShader(basicShader);
	triangle->Draw();
}

#pragma endregion




