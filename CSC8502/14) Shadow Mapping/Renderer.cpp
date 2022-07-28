#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"

#define SHADOWSIZE 2048

Renderer::Renderer(Window& parent) :OGLRenderer(parent)
{
	camera	= new Camera(-30.0f, 315.0f, Vector3(-8.0f, 5.0f, 8.0f));
	light	= new Light(Vector3(-20.0f, 10.0f, -20.0f), Vector4(1, 1, 1, 1), 250.0f);
	
	sceneShader		= new Shader("shadowSceneVert.glsl", "shadowSceneFrag.glsl");
	shadowShader	= new Shader("shadowVert.glsl", "shadowFrag.glsl");

	if (!sceneShader->LoadSuccess() || !shadowShader->LoadSuccess())
		return;

	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);	
	glDrawBuffer(GL_NONE);

	sceneMeshes.emplace_back(Mesh::GenerateQuad());
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Sphere.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Cone.msh"));

	sceneDiffuse = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sceneBump	 = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	SetTextureRepeating(sceneDiffuse, true);
	SetTextureRepeating(sceneBump, true);
	glEnable(GL_DEPTH_TEST);
	sceneTransforms.resize(4);
}

Renderer::~Renderer(void)
{
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);

	for (auto& i : sceneMeshes)
	{
		delete i;
	}

	delete camera;
	delete shadowShader;
}

void Renderer::UpdateScene(float dt)
{
	camera->UpdateCamera(dt);
	sceneTime += dt;
	light->Update(dt);

	for (int i = 1; i < 4; i++)
	{
		Vector3 t = Vector3(-10 + (5 * i), 2.0f + std::sin(sceneTime * i), 0);
		sceneTransforms[i] = Matrix4::Translation(t) * Matrix4::Rotation(sceneTime * 10 * i, Vector3(1, 0, 0));
	}
}

void Renderer::RenderScene()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawMainScene();
}

void Renderer::DrawShadowScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	BindShader(shadowShader);

	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(0, 0, 0));
	projMatrix = Matrix4::Perspective(1, 100, 1, 45);
	shadowMatrix = projMatrix * viewMatrix;

	for (int i = 0; i < 4; i++)
	{
		modelMatrix = sceneTransforms[i];
		UpdateShaderMatrices();
		sceneMeshes[i]->Draw();
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawMainScene()
{
	BindShader(sceneShader);
	SetShaderLight(*light);

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex"), 2);
	glUniform3fv(glGetUniformLocation(sceneShader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneDiffuse);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, sceneBump);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	for (int i = 0; i < 4; ++i)
	{
		modelMatrix = sceneTransforms[i];
		UpdateShaderMatrices();
	}
}