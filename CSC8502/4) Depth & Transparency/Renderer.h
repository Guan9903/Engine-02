#pragma once

#include "../nclgl/OGLRenderer.h"

class Renderer :public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;

	void ToggleObject();
	void ToggleDepth();
	void ToggleAlphaBlend();
	void ToggleBlendMode();
	void MoveObject(float by);

protected:
	Shader* shader;
	Mesh* meshes[2];
	GLuint textures[2];
	Vector3 position[2];

	bool modifyObject;
	bool usingDepth;
	bool usingAlpha;
	int  blendMode;
};