#include "Renderer.h"
#include "../nclgl/CubeRobot.h"
#include <algorithm>

#define SHADOWSIZE 4096

Renderer::Renderer(Window &parent) : OGLRenderer(parent)	
{
	Variables();
	LoadFiles();
	LoadModels();
	GenerateShader();
	ShadowMapping();
	//BufferSetting();
	GLcommands();
	SwitchToPerspective();

	init = true;
}

Renderer::~Renderer(void)	{

	delete skyQuad;
	delete heightMap;
	delete camObj;
	delete dirLight;
	delete dirLightShader;
	delete skyBoxShader;
	delete waterShader;

	for (auto& i : sceneMeshes)
	{
		delete i;
	}
	delete root;

	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
}

/*Initialize variables*/
void Renderer::Variables()
{
	/*Stack*/
	waterRotate				= 0.0f;
	waterCycle				= 0.0f;
	sceneTime				= 0.0f;
	frameTime				= 0.0f;
	currentFrame			= 0;

	/*Heap*/
	skyQuad					= Mesh::GenerateQuad();
	heightMap				= new HeightMap(TEXTUREDIR_HEIGHTMAPS"heightmap07.png");
	heightMapSize			= heightMap->GetHeightMapSize();
	camObj					= new Camera(-15.0f, -45.0f, heightMapSize * Vector3(0.3f, 5.0f, 0.9f));
	mapCamObj				= new Camera(-90.0f, 0.0f, heightMapSize * Vector3(0.5f, 20.0f, 0.5f));

	dirLight				= new Light(Vector4(1, 1, 1, 1), Vector3(20.0f, -50.0f, 20.0f));

	/*Scene Tree*/
	root					= new SceneNode();
	
}

void Renderer::LoadFiles()
{
	waterTex		= SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	waterBumpTex	= SOIL_load_OGL_texture(TEXTUREDIR"waterbump.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex		= SOIL_load_OGL_texture(TEXTUREDIR_HEIGHTMAPS"terrianMap01.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump		= SOIL_load_OGL_texture(TEXTUREDIR_NORMALMAPS"normalMap01.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	treeTexture		= SOIL_load_OGL_texture(TEXTUREDIR_MODELS"tree01.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	dayCubeMap		= SOIL_load_OGL_cubemap(TEXTUREDIR_SKYBOXES"1left.png", TEXTUREDIR_SKYBOXES"1right.png",
											TEXTUREDIR_SKYBOXES"1up.png", TEXTUREDIR_SKYBOXES"1down.png",
											TEXTUREDIR_SKYBOXES"1front.png", TEXTUREDIR_SKYBOXES"1back.png",
											SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	nightCubeMap	= SOIL_load_OGL_cubemap(TEXTUREDIR_SKYBOXES"2left.png", TEXTUREDIR_SKYBOXES"2right.png",
											TEXTUREDIR_SKYBOXES"2up.png", TEXTUREDIR_SKYBOXES"2down.png",
											TEXTUREDIR_SKYBOXES"2front.png", TEXTUREDIR_SKYBOXES"2back.png",
											SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!earthTex || 
		!earthBump || 
		!waterTex ||
		!waterBumpTex ||
		!treeTexture ||
		!dayCubeMap ||
		!nightCubeMap)
		return;

	SetTextureRepeating(earthTex,		true);
	SetTextureRepeating(earthBump,		true);
	SetTextureRepeating(waterTex,		true);
	SetTextureRepeating(waterBumpTex,	true);
}

void Renderer::GenerateShader()
{
	skyBoxShader		= new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	waterShader			= new Shader("waterVert.glsl", "waterFrag.glsl");
	dirLightShader		= new Shader("lightsVert.glsl", "lightsFrag.glsl");
	shadowShader		= new Shader("shadowVert.glsl", "shadowFrag.glsl");
	sceneShadowShader	= new Shader("shadowSceneVert.glsl", "shadowSceneFrag.glsl");
	animModelShader		= new Shader("skinningVertex.glsl", "texturedFragment.glsl");

	if (!skyBoxShader->LoadSuccess() ||
		!waterShader->LoadSuccess() ||
		!dirLightShader->LoadSuccess() ||
		!shadowShader->LoadSuccess() ||
		!sceneShadowShader->LoadSuccess() ||
		!animModelShader->LoadSuccess())
		return;

	/*Deferred rendering test*/
	sceneShader		= new Shader("gBufferVert.glsl", "gBufferFrag.glsl");
	lightingShader	= new Shader("lightBufferVert.glsl", "lightBufferFrag.glsl");
	combineShader	= new Shader("combineVert.glsl", "combineFrag.glsl");
}

/*OpenGL commands*/
void Renderer::GLcommands()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void Renderer::UpdateScene(float dt) 
{
	camObj->UpdateCamera(dt);
	/*for (int i = 0; i < 4; i++)
	{
		mapCamObjs[i]->UpdateCamera(dt);
	}*/
	//mapCamObj->UpdateMapCamera(camObj, dt);
	//viewMatrix = camObj->BuildViewMatrix();
	//frameFrustum.FromMatrix(projMatrix * viewMatrix);
	dirLight->UpdateDirLight(dt, 20.0f, Vector3(1, 0, 0));
	//std::cout << dirLight->GetDirection() << std::endl;

	waterRotate		+= dt * 2.0f;
	waterCycle		+= dt * 0.25f;
	sceneTime		+= dt;

	/*Animation*/
	frameTime -= dt;
	while (frameTime < 0.0f)
	{
		currentFrame = (currentFrame + 1) % anim->GetFrameCount();
		frameTime += 1.0f / anim->GetFrameRate();
	}

	root->Update(dt);
}

void Renderer::RenderScene()
{
	//glClearColor(0.2f,0.2f,0.2f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (isSplit)
	{
		glViewport(0, 0, width / 2, height / 2);
		//DrawSkybox();
		DrawShadow();
		viewMatrix = mapCamObj->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
		DrawHeightmap();
		DrawWater();

	}
	else
	{
		//glViewport(0, 0, width, height);
		DrawSkybox();
		DrawShadow();
		viewMatrix = camObj->BuildViewMatrix();
		DrawHeightmap();
		DrawWater();
		DrawAnimModel();
		DrawNodes();
		ClearNodeLists();
	}


	//FillBuffers();	//DrawLights();	//CombineBuffers();

}

Matrix4 Renderer::SetTransform(Vector3 translation, Vector3 scale, float rotateDegree, Vector3 rotateAxis)
{
	return Matrix4::Translation(translation) * Matrix4::Scale(scale) * Matrix4::Rotation(rotateDegree, rotateAxis);
}

#pragma region Models

void Renderer::LoadModels()
{
	/*Load Models*/
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Sphere.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Palm_Tree.msh"));
	sceneTransforms.resize(2);
	sceneTransforms[0] = SetTransform(heightMapSize * Vector3(0.65, 0.80, 0.5), Vector3(100, 100, 100), 0, Vector3(1, 0, 0));
	sceneTransforms[1] = SetTransform(heightMapSize * Vector3(0.4, 0.90, 0.6), Vector3(100, 100, 100), 0, Vector3(1, 0, 0));
	SceneNode* tree = new SceneNode(sceneMeshes[1], Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	tree->SetTransform(sceneTransforms[0]);
	root->AddChild(tree);

	/*Load Animation*/
	animModelMesh		= Mesh::LoadFromMeshFile("Role_B.msh");
	animMaterial		= new MeshMaterial("Role_B.mat");
	anim				= new MeshAnimation("Role_B.anm");
	for (int i = 0; i < animModelMesh->GetSubMeshCount(); i++)
	{
		const MeshMaterialEntry* matEntry = animMaterial->GetMaterialForLayer(i);
		const string* filename = nullptr;

		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint textID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTexture.emplace_back(textID);
	}
	SceneNode* role = new SceneNode(animModelMesh, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	role->SetTransform(sceneTransforms[1]);
	root->AddChild(role);
}

void Renderer::DrawAnimModel()
{
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	BindShader(animModelShader);	glUniform1i(glGetUniformLocation(animModelShader->GetProgram(), "diffuseTex"), 0);
	modelMatrix = sceneTransforms[1];
	UpdateShaderMatrices();

	vector<Matrix4> frameMatrices;

	const Matrix4* invBindPose	= animModelMesh->GetInverseBindPose();
	const Matrix4* frameData	= anim->GetJointData(currentFrame);

	for (unsigned int i = 0; i < animModelMesh->GetJointCount(); i++)
	{
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(animModelShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	for (int i = 0; i < animModelMesh->GetSubMeshCount(); i++)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTexture[i]);
		animModelMesh->DrawSubMesh(i);
	}
}

void Renderer::DrawTree()
{

}

#pragma endregion

#pragma region Camera

void Renderer::SwitchToAuto()
{
	if(camObj->isAuto)
		camObj->isAuto = false;
	else
		camObj->isAuto = true;
}

void Renderer::SwitchToPerspective()
{
	projMatrix = Matrix4::Perspective(1.0f, 20000.0f, (float)width / (float)height, 60.0f);
}

void Renderer::SwitchToOrthographic()
{
	projMatrix = Matrix4::Orthographic(-1.0f, 20000.0f, width / 2.0f, -width / 2.0f, height / 2.0f, -height / 2.0f);
}

void Renderer::GenerateMapTex()
{
	/*Map Texture*/
	glGenTextures(1, &mapTex);
	glBindTexture(GL_TEXTURE_2D, mapTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	/*Map FBO*/
	glGenFramebuffers(1, &mapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mapFBO, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	viewMatrix = mapCamObj->BuildViewMatrix();
}

#pragma endregion

#pragma region Node

void Renderer::BuildNodeLists(SceneNode* from)
{
	if (frameFrustum.InsideFrustum(*from))
	{
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camObj->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f)
			treesNodeList.push_back(from);
		else
			nodeList.push_back(from);
	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); i++)
	{
		BuildNodeLists((*i));
	}
}

void Renderer::SortNodeLists()
{
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::DrawNodes()
{
	for (const auto& i : nodeList)
	{
		DrawNode(i);
	}
	for (const auto& i : treesNodeList)
	{
		DrawNode(i);
	}
}

void Renderer::DrawNode(SceneNode* n)
{
	if (n->GetMesh())
	{
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(glGetUniformLocation(nodeShader->GetProgram(), "modelMatrix"), 1, false, model.values);
		glUniform4fv(glGetUniformLocation(nodeShader->GetProgram(), "nodeColour"), 1, (float*)& n->GetColour());

		nodeTexture = n->GetTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, nodeTexture);

		glUniform1i(glGetUniformLocation(nodeShader->GetProgram(), "useTexture"), nodeTexture);

		n->Draw(*this);
	}
}

void Renderer::ClearNodeLists()
{
	treesNodeList.clear();
	nodeList.clear();
}

#pragma endregion

#pragma region SkyBox

void Renderer::DrawSkybox()
{
	glDepthMask(GL_FALSE);

	BindShader(skyBoxShader);
	UpdateShaderMatrices();

	skyQuad->Draw();
	glDepthMask(GL_TRUE);
}

#pragma endregion

#pragma region Terrain

void Renderer::DrawHeightmap()
{
	BindShader(dirLightShader);
	SetShaderDirectionalLight(*dirLight);
	//viewMatrix = camObj->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glUniform1i(glGetUniformLocation(dirLightShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(dirLightShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(dirLightShader->GetProgram(), "shadowTex"), 2);
	glUniform1f(glGetUniformLocation(dirLightShader->GetProgram(), "shininess"), 16.0f);
	glUniform3fv(glGetUniformLocation(dirLightShader->GetProgram(), "cameraPos"), 1, (float*)& camObj->GetPosition());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	UpdateShaderMatrices();
	heightMap->Draw();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, treeTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	modelMatrix = sceneTransforms[0];
	UpdateShaderMatrices();
	sceneMeshes[1]->Draw();

	/*BindShader(spotLightShader);
	SetShaderDirectionalLight(*spotLight);

	glUniform1i(glGetUniformLocation(spotLightShader->GetProgram(), "diffuseTex"), 0);
	glUniform1f(glGetUniformLocation(spotLightShader->GetProgram(), "shininess"), 16.0f);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(spotLightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();
	UpdateShaderMatrices();

	heightMap->Draw();*/

}

void Renderer::DrawWater()
{
	BindShader(waterShader);
	glUniform3fv(glGetUniformLocation(waterShader->GetProgram(), "cameraPos"), 1, (float*)& camObj->GetPosition());
	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, nightCubeMap);

	Vector3 hSize = heightMap->GetHeightMapSize();

	modelMatrix = Matrix4::Translation(hSize * Vector3(0.5f, 0.2f, 0.5f)) *
		Matrix4::Scale(hSize * 0.5f) *
		Matrix4::Rotation(90, Vector3(1, 0, 0));

	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
		Matrix4::Scale(Vector3(10, 10, 10)) *
		Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	skyQuad->Draw();
}

#pragma endregion

#pragma region Shadow

void Renderer::ShadowMapping()
{
	/*Shadow Texture*/
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	/*Shadow FBO*/
	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawShadow()
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	BindShader(shadowShader);

	viewMatrix		= Matrix4::BuildViewMatrix(dirLight->GetPosition(), Vector3(0, 0, 0));
	projMatrix		= Matrix4::Orthographic(1, 50000, -10000, 10000, -10000, 10000);
	//projMatrix		= Matrix4::Perspective(1, 20000, 1, 45);
	shadowMatrix	= projMatrix * viewMatrix;

	modelMatrix.ToIdentity();
	UpdateShaderMatrices();
	heightMap->Draw();

	modelMatrix = sceneTransforms[0];
	UpdateShaderMatrices();
	sceneMeshes[1]->Draw();

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#pragma endregion

#pragma region G-Buffer

void Renderer::BufferSetting()
{
	glGenFramebuffers(1, &bufferFBO);	glGenFramebuffers(1, &LightingFBO);
	GLenum buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	//Generate scene depth texture
	GenerateScreenTexture(bufferDepthTex, true);
	GenerateScreenTexture(bufferColourTex);	GenerateScreenTexture(bufferNormalTex);
	//GenerateScreenTexture(bufferShadowTex, true);
	GenerateScreenTexture(lightDiffuseTex);
	GenerateScreenTexture(lightSpecularTex);

	//Attach them to FBOs
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bufferNormalTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferShadowTex, 0);
	glDrawBuffers(2, buffers);	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, LightingFBO);	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightDiffuseTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lightSpecularTex, 0);
	glDrawBuffers(2, buffers);	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)		return;	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);	glEnable(GL_DEPTH_TEST);	//glEnable(GL_CULL_FACE);	glEnable(GL_BLEND);
}

void Renderer::GenerateScreenTexture(GLuint& into, bool depth)
{
	glGenTextures(1, &into);	glBindTexture(GL_TEXTURE_2D, into);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint format = depth ? GL_DEPTH_COMPONENT24 : GL_RGBA8;	GLuint type = depth ? GL_DEPTH_COMPONENT : GL_RGBA;

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, type, GL_UNSIGNED_BYTE, NULL);	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::FillBuffers()
{
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(sceneShader);	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "bumpTex"), 1);

	glActiveTexture(GL_TEXTURE0);	glBindTexture(GL_TEXTURE_2D, earthTex);
	glActiveTexture(GL_TEXTURE1);	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity();	viewMatrix = camObj->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	UpdateShaderMatrices();
	heightMap->Draw();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawLights()
{
	glBindFramebuffer(GL_FRAMEBUFFER, LightingFBO);	BindShader(lightingShader);
	glClearColor(0, 0, 0, 1);	glClear(GL_COLOR_BUFFER_BIT);	glBlendFunc(GL_ONE, GL_ONE);
	//glCullFace(GL_FRONT);
	glDepthFunc(GL_ALWAYS);	glDepthMask(GL_FALSE);
	glUniform1i(glGetUniformLocation(lightingShader->GetProgram(), "depthTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glUniform1i(glGetUniformLocation(lightingShader->GetProgram(), "normTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bufferNormalTex);

	glUniform3fv(glGetUniformLocation(lightingShader->GetProgram(), "cameraPos"), 1, (float*)& camObj->GetPosition());
	glUniform2f(glGetUniformLocation(lightingShader->GetProgram(), "pixelSize"), 1.0f / width, 1.0f / height);

	Matrix4 invViewProj = (projMatrix * viewMatrix).Inverse();	glUniformMatrix4fv(glGetUniformLocation(lightingShader->GetProgram(), "inverseProjView"), 1, false, invViewProj.values);
	UpdateShaderMatrices();
	SetShaderDirectionalLight(*dirLight);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//glCullFace(GL_BACK);	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glClearColor(0.2f, 0.2f, 0.2f, 1);	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::CombineBuffers()
{
	BindShader(combineShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex);	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "diffuseLight"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightDiffuseTex);	glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "specularLight"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, lightSpecularTex);
	screenQuad->Draw();
}

#pragma endregion
