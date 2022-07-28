#pragma once
#include "../nclgl/OGLRenderer.h"

class HeightMap;
class Camera;
class Shader;

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene()			override;
	void UpdateScene(float dt)	override;

protected:
	void		DrawHeightmap();
	void		DrawWater();
	void		DrawSkybox();

	Shader*		lightShader;

	HeightMap*	heightMap;
	
	GLuint		cubeMap;
	GLuint		waterTex;
	GLuint		earthTex;
	GLuint		earthBump;

	float		waterRotate;
};