#include "Renderer.h"

Renderer::Renderer(Window& parent)
	:OGLRenderer(parent)
{
	triangle = Mesh::GenerateTriangle();

	texture1 = SOIL_load_OGL_texture(TEXTUREDIR"brick.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (!texture1) return;

	texture2 = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	if (!texture2) return;

	shader = new Shader("TexturedVertex.glsl", "texturedfragment.glsl");

	if (!shader->LoadSuccess()) return;

	camera = new Camera();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	filtering = true;
	repeating = false;
	mixFactor = 0.0f;
	init = true;
}
Renderer::~Renderer(void)
{
	delete triangle;
	delete shader;
	glDeleteTextures(1, &texture1);
	glDeleteTextures(1, &texture2);
}

void Renderer::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	BindShader(shader);
	modelMatrix = Matrix4::Translation(Vector3(0.0f, 0.0f, -5.0f));
	UpdateShaderMatrices();

	glUniform1f(glGetUniformLocation(shader->GetProgram(), "mixFactor"), mixFactor);
	
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "glassTex"), 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	

	triangle->Draw();
}

void Renderer::UpdateTextureMatrix(float value)
{
	//pop, rotate, push: we want to rotate around the centre of the texture, so we're moving its centre to the origin
	Matrix4 push = Matrix4::Translation(Vector3(-0.5f, -0.5f, 0));
	Matrix4 pop = Matrix4::Translation(Vector3(0.5f, 0.5f, 0));
	Matrix4 rotation = Matrix4::Rotation(value, Vector3(0, 0, 1));

	textureMatrix = pop * rotation * push;
}

void Renderer::ToggleRepeating()
{
	repeating = !repeating;
	SetTextureRepeating(texture1, repeating);
	SetTextureRepeating(texture2, repeating);
}

void Renderer::ToggleFiltering()
{
	filtering = !filtering;
	glBindTexture(GL_TEXTURE_2D, texture1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering ? GL_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering ? GL_LINEAR : GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering ? GL_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering ? GL_LINEAR : GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::UpdateScene(float dt)
{
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
}