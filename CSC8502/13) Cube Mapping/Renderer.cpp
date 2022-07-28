#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/Camera.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent)
{
	quad = Mesh::GenerateQuad();

	heightMap = new HeightMap(TEXTUREDIR"noise.png");

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg",
									TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg",
									TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg",
									SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!earthTex || !earthBump || !cubeMap || !waterTex)
		return;

	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
	SetTextureFiltering(earthTex, true);
	SetTextureFiltering(earthBump, true);

	reflectShader	= new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader	= new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader		= new Shader("lightsVert.glsl", "lightsFrag.glsl");
	//lightShader = new Shader("lightsVert.glsl", "lightsFrag.glsl");

	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !lightShader->LoadSuccess())
		return;

	Vector3 heightmapSize = heightMap->GetHeightMapSize();
	camera = new Camera(-45.0f, 0.0f, heightmapSize * Vector3(0.5f, 5.0f, 0.5f));
	//light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1, 1, 1, 1), heightmapSize.x);
	light = new Light(Vector4(1, 1, 1, 1), Vector3(-0.2f, -0.3f, -0.3f));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	waterCycle = 0.0f;
}

Renderer ::~Renderer(void)
{
	delete camera;
	delete quad;
	delete skyboxShader;
}

void Renderer::UpdateScene(float dt)
{
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	waterCycle	+= dt * 0.25f;
}

void Renderer::RenderScene()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkybox();
	DrawWater();
}

void Renderer::DrawSkybox()
{
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();
	quad->Draw();
	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap()
{
	BindShader(lightShader);
	SetShaderDirectionalLight(*light);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	heightMap->Draw();
}

void Renderer::DrawWater()
{
	BindShader(reflectShader);
	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);

	glActiveTexture(GL_TEXTURE2);

	Vector3 hSize = heightMap->GetHeightMapSize();

	modelMatrix   =	Matrix4::Translation(hSize * 0.5f) *
					Matrix4::Scale(hSize * 0.5f) *
					Matrix4::Rotation(90, Vector3(1, 0, 0));

	textureMatrix =	Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
					Matrix4::Scale(Vector3(10, 10, 10)) *
					Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
}