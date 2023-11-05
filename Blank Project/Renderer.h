#pragma once
#include "../nclgl/OGLRenderer.h"


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

	HeightMap* heightMap;
	Mesh* quad;
	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Camera* camera;
	Light* light;
	GLuint heightmapTex;
	GLuint heightmapBump;
	GLuint cubeMap;
};