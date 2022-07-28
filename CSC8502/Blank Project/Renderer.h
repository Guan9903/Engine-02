#pragma once
#include "../NCLGL/OGLRenderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Light.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"


class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene()				override;
	void UpdateScene(float msec)	override;

	void Variables();
	void LoadFiles();
	void GenerateShader();
	void ShadowMapping();
	void GenerateMapTex();
	void GLcommands();

protected:
	Matrix4 SetTransform(Vector3 translation = Vector3(2048, 500, 2048),
		Vector3 scale = Vector3(100, 100, 100),
		float rotateDegree = 0.0f,
		Vector3 rotationAxis = Vector3(0, 1, 0));

	Vector3 heightMapSize;
	float	sceneTime;
	float	frameTime;
	int		currentFrame;

#pragma region Camera

public:
	void		SwitchToAuto();
	void		SwitchToPerspective();
	void		SwitchToOrthographic();
	inline void SetScale(float s) { camScale = s; }
	inline void SetRotation(float r) { camRotation = r; }
	inline void SetPosition(Vector3 p) { camPosition = p; }
	bool		isSplit = false;

protected:
	float		camScale;
	float		camRotation;
	Vector3		camPosition;
	Camera*		camObj;
	Camera*		mapCamObj;
	Frustum		frameFrustum;

	GLuint		mapTex;
	GLuint		mapFBO;

#pragma endregion

#pragma region Models

protected:
	void			LoadModels();
	void			DrawAnimModel();
	void			DrawTree();
	Mesh* animModelMesh;
	Shader* animModelShader;
	MeshMaterial* animMaterial;
	MeshAnimation* anim;
	vector<GLuint>	matTexture;

	GLuint			treeTexture;

#pragma endregion

#pragma region Node

protected:
	void		BuildNodeLists(SceneNode* from);
	void		SortNodeLists();
	void		ClearNodeLists();
	void		DrawNodes();
	void		DrawNode(SceneNode* n);
	Shader* nodeShader;
	GLuint		nodeTexture;
	SceneNode* root;
	Mesh* cube;

	vector<SceneNode*> modelsNodeList;
	vector<SceneNode*> treesNodeList;
	vector<SceneNode*> nodeList;

#pragma endregion

#pragma region Skybox

protected:
	void		DrawSkybox();
	Shader* skyBoxShader;
	GLuint		dayCubeMap;
	GLuint		nightCubeMap;
	Mesh* skyQuad;

#pragma endregion

#pragma region Terrain

protected:
	void		DrawHeightmap();
	void		DrawWater();

	Shader* waterShader;
	HeightMap* heightMap;
	GLuint		terrianTex;
	GLuint		waterTex;
	GLuint		waterBumpTex;
	GLuint		earthTex;
	GLuint		earthBump;

	float		waterRotate;
	float		waterCycle;

#pragma endregion

#pragma region Light

protected:
	Light* dirLight;			//directional lighting
	Shader* dirLightShader;

#pragma endregion

#pragma region Shadow

protected:
	void		DrawShadow();

	GLuint		shadowTex;
	GLuint		shadowFBO;

	Shader* sceneShadowShader;
	Shader* shadowShader;

	vector<Mesh*>	sceneMeshes;
	vector<Matrix4> sceneTransforms;

#pragma endregion

#pragma region Post Processing



#pragma endregion


#pragma region G-Buffer

public:
	void		BufferSetting();

protected:
	void		FillBuffers();
	void		DrawLights();
	void		CombineBuffers();
	void		GenerateScreenTexture(GLuint& into, bool depth = false);

	Shader*		sceneShader;
	Shader*		lightingShader;
	Shader*		combineShader;

	GLuint		bufferFBO;
	GLuint		bufferColourTex;
	GLuint		bufferNormalTex;
	GLuint		bufferDepthTex;
	GLuint		bufferShadowTex;

	GLuint		LightingFBO;
	GLuint		lightDiffuseTex;
	GLuint		lightSpecularTex;

	Mesh*		screenQuad;

#pragma endregion

};
