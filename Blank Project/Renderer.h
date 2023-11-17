#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/Light.h"


class HeightMap;
class Camera;
class Light;
class Shader;

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
	void Renderer::BuildNodeLists(SceneNode* from);
	void Renderer::DrawShadowScene();

	HeightMap* heightMap;
	Mesh* quad;
	Shader* lightShader;
	Shader* shadowShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	SceneNode* root;
	SceneNode* sun;
	Camera* camera;
	PointLight* light;
	DirectionLight* sunLight;
	Spotlight* spotlight;
	GLuint heightmapTex;
	GLuint heightmapBump;
	GLuint cubeMap;
	GLuint currentTexture;
	GLuint shadowTex;
	GLuint shadowFBO;
	float sceneTime;
	Frustum frameFrustum;
	Matrix4 worldTransform;
	Vector3 pointToSun;

	vector<SceneNode*> nodeList;
};