#pragma once
#include "../nclgl/OGLRenderer.h"


class HeightMap;
class Camera;
class PointLight;
class Shader;

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;
protected:
	HeightMap* heightMap;
	Shader* shader;
	Camera* camera;
	PointLight* light;
	GLuint texture;
	GLuint bumpmap;
};