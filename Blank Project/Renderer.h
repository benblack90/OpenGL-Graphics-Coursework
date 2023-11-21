#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/Light.h"


class HeightMap;
class Camera;
class Light;
class Shader;

struct ShadowMapTex
{
	GLuint shadowTex;
	GLuint shadowFBO;
	Matrix4 shadowMatrix;
};

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;
protected:
	void LoadTerrain();
	void LoadCubeMap();
	void LoadPostProcess();
	void DrawSkyBox();
	void DrawHeightMap();
	void DrawNode(SceneNode* n);
	void DrawMultiPassPostProcess();
	void DrawSinglePassPostProcess();
	void PresentScene();
	void BuildNodeLists(SceneNode* from);
	void DrawShadowScene();
	void GenerateShadowFBOs();
	void PlaceMesh(float x, float z, GLuint albedo, GLuint bump, Shader* shader, std::string meshName);
	void ResetViewProjToCamera();
	void DrawIce();

	HeightMap* heightMap;
	Mesh* quad;
	Mesh* iceQuad;
	Shader* lightShader;
	Shader* shadowShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* rockShader;
	Shader* pProcShader;
	Shader* pProcOutShader;
	SceneNode* root;
	SceneNode* sun;
	Camera* camera;
	DirectionLight* sunLight;
	Spotlight* spotlight;
	GLuint heightmapTexSand;
	GLuint heightmapBump;
	GLuint cubeMap;
	GLuint rockAlbedo;
	GLuint rockBump;
	GLuint iceAlbedo; 
	GLuint iceBump;
	GLuint bufferFBO;
	GLuint processFBO;
	GLuint hdrFBO;
	GLuint fpColourTex;
	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;
	ShadowMapTex shMapTex[2];
	float sceneTime;
	float horizonCheck;
	float signalLoss;
	Frustum frameFrustum;
	Matrix4 worldTransform;
	Vector3 pointToSun;
	Vector3 pointToSatellite;

	vector<SceneNode*> nodeList;
	vector<Light*> lights;
};