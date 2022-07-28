#pragma once

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Camera.h"

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene()			override;
protected:
	void		PresentScene();
	void		DrawPostProcess();
	void		DrawScene();
	Shader*		sceneShader;
	Shader*		processShader;

	Camera*		camera;
	Mesh*		quad;

	GLuint		heightTexture;
	GLuint		bufferFBO;
	GLuint		bufferDepthTex;
};