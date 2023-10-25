#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Camera.h"

class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	virtual ~Renderer(void);

	virtual void RenderScene();
	void UpdateScene(float dt);

	void UpdateTextureMatrix(float rotation);
	void ToggleRepeating();
	void ToggleFiltering();
	void setMixFactor(float m) { mixFactor = m; }
protected:
	Shader* shader;
	Mesh* triangle;
	Camera* camera;
	GLuint texture1;
	GLuint texture2;
	bool filtering;
	bool repeating;
	float mixFactor;
};