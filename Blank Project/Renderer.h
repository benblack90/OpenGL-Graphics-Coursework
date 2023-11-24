#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/Light.h"
#include "SolarSystem.h"


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
	void CheckSceneControlKeys();
	void LoadTerrain();
	void LoadCubeMap();
	void LoadPostProcess();
	void LoadSun();
	void LoadHudTexes();
	void DrawSkyBox();
	void DrawHeightMap();
	void DrawNode(SceneNode* n);
	void DrawMultiPassBlur();
	void DrawFilmGrainPass();
	void DrawBloom();
	void HDRToneMap();
	void PresentScene();
	void BuildNodeLists(SceneNode* from);
	void DrawShadowScene();
	void GenerateShadowFBOs();
	void PlaceMesh(float x, float z, GLuint albedo, GLuint bump, Shader* shader, std::string meshName);
	void ResetViewProjToCamera();
	void DrawIce();
	void DrawHud(float dt);

	bool planetSide;
	bool autoCamera;
	bool showHud;
	bool showNoise;
	bool toneMapping;
	bool showBloom;
	HeightMap* heightMap;
	Mesh* quad;
	Shader* lightShader;
	Shader* shadowShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* rockShader;
	Shader* pProcBloomExtract;
	Shader* pProcGrainShader;
	Shader* pProcHDRShader;
	Shader* pProcOutShader;
	Shader* pProcBlurShader;
	Shader* pProcBloomAdd;
	SceneNode* planetRoot;
	SolarSystem* orbitRoot;
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
	GLuint fullSigTex;
	GLuint hiMidSigTex;
	GLuint loMidSigTex;
	GLuint loSigWarnTexes[2];
	bool warnText;
	GLuint bufferFBO;
	GLuint processFBO;
	GLuint blurColourTex[2];
	GLuint bufferTex;
	GLuint bufferDepthTex;
	ShadowMapTex shMapTex[2];
	float sceneTime;
	float deltaTime;
	float hudTimer;
	float horizonCheck;
	float signalLoss;
	Frustum frameFrustum;
	Matrix4 worldTransform;
	Vector3 pointToSun;
	Vector3 pointToSatellite;

	vector<SceneNode*> nodeList;
	vector<Light*> lights;
};