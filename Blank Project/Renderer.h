#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"


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

	HeightMap* heightMap;
	Mesh* quad;
	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* rockShader;
	SceneNode* root;
	Camera* camera;
	Light* light;
	GLuint heightmapTex;
	GLuint heightmapBump;
	GLuint cubeMap;
	GLuint currentTexture;
	Frustum frameFrustum;
	Matrix4 worldTransform;
};