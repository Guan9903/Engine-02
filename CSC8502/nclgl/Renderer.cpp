#include "Renderer.h"

Renderer::Renderer(Window & parent) : OGLRenderer(parent)
{
	triangle = Mesh::GenerateTriangle();

#pragma region task 1

	/*basicShader = new Shader("basicVertex.glsl", "colourFragment.glsl");
	if (!basicShader->LoadSuccess())
		return;

	init = true;*/

#pragma endregion

#pragma region task 2

	matrixShader = new Shader("", "");
	if (!matrixShader->LoadSuccess())
		return;

	init = true;

	SwitchToOrthographic();

	cameraObj = new Camera();

#pragma endregion

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

#pragma region task 2

void Renderer::UpdateScene(float dt)
{
	cameraObj->UpdateCamera(dt);
	viewMatrix = cameraObj->BuildViewMatrix();
}

#pragma endregion



