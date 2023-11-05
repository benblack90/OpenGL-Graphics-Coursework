#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"

Renderer::Renderer(Window& parent)
	:OGLRenderer(parent)
{
	quad = Mesh::GenerateQuad();
	LoadTerrain();
	LoadCubeMap();

	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	camera = new Camera(-45.0f, 0.0f, 0, heightmapSize * Vector3(0.5f, 1.0f, 0.5f));

	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1, 1, 1, 1), heightmapSize.x * 0.5f);

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	init = true;
}

Renderer::~Renderer()
{
	delete camera;
	delete heightMap;
	delete lightShader;
	delete skyboxShader;
	delete light;
	delete quad;
}

void Renderer::LoadCubeMap()
{
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.JPG",
		TEXTUREDIR"rusted_east.JPG",
		TEXTUREDIR"rusted_up.JPG", TEXTUREDIR"rusted_down.JPG",
		TEXTUREDIR"rusted_south.JPG", TEXTUREDIR"rusted_north.JPG",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	if ( !skyboxShader->LoadSuccess() || !cubeMap) return;
}

void Renderer::LoadTerrain()
{
	heightMap = new HeightMap(TEXTUREDIR"heightmapfjord.png");
	heightmapTex = SOIL_load_OGL_texture(TEXTUREDIR"TCom_Sand_Muddy2_2x2_512_albedo.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	heightmapBump = SOIL_load_OGL_texture(TEXTUREDIR"TCom_Sand_Muddy2_2x2_512_roughness.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	lightShader = new Shader("perPixelVertex.glsl", "perPixelFragment.glsl");
	if (!lightShader->LoadSuccess() || !heightmapTex || !heightmapBump) return;

	SetTextureRepeating(heightmapTex, true);
	SetTextureRepeating(heightmapBump, true);
}

void Renderer::UpdateScene(float dt)
{
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderScene()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkyBox();
	DrawHeightMap();
}

void Renderer::DrawHeightMap()
{
	BindShader(lightShader);
	SetShaderLight(*light);

	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmapTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightmapBump);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	heightMap->Draw();
}

void Renderer::DrawSkyBox()
{
	glDepthMask(GL_FALSE);
	BindShader(skyboxShader);
	UpdateShaderMatrices();
	quad->Draw();
	glDepthMask(GL_TRUE);
}