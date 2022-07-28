#pragma once
#include "../nclgl/OGLRenderer.h"

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;

protected:
	Mesh* triangle;
	Shader* basicShader;
};