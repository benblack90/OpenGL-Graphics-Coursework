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
	void DrawSkyBox();
	void DrawHeightMap();
	void DrawNode(SceneNode* n);
	void BuildNodeLists(SceneNode* from);
	void DrawShadowScene();
	void GenerateShadowFBOs();

	HeightMap* heightMap;
	Mesh* quad;
	Shader* lightShader;
	Shader* shadowShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	SceneNode* root;
	SceneNode* sun;
	Camera* camera;
	DirectionLight* sunLight;
	Spotlight* spotlight;
	GLuint heightmapTex;
	GLuint heightmapBump;
	GLuint cubeMap;
	GLuint currentTexture;
	ShadowMapTex shMapTex[2];
	float sceneTime;
	Frustum frameFrustum;
	Matrix4 worldTransform;
	Vector3 pointToSun;
	//NB sunlight originates from a different point than the sun model, because the sun model moves relative to the camera
	Vector3 sunlightOrigin;

	vector<SceneNode*> nodeList;
	vector<Light*> lights;
};