#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"

Renderer::Renderer(Window& parent)
	:OGLRenderer(parent)
{
	quad = Mesh::GenerateQuad();
	LoadTerrain();
	

	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	camera = new Camera(-15.0f, 0.0f, 0, heightmapSize * Vector3(0.5f, 1.0f, 0.5f));

	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1, 1, 1, 1), heightmapSize.x * 0.5f);

	//load rock shader
	root = new SceneNode();
	rockShader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	if (!rockShader->LoadSuccess()) return;
	SceneNode* s = new SceneNode();
	s->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	s->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	s->SetTransform(Matrix4::Translation(heightmapSize * Vector3(0.5f, 1.0f, 0.5f)));
	s->SetModelScale(Vector3(1.0f, 1.0f, 1.0f));
	s->SetBoundingRadius(1.0f);
	s->SetMesh(Mesh::LoadFromMeshFile("rock_01.msh"));
	root->AddChild(s);

	LoadCubeMap();
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
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"starbox_right.png",
		TEXTUREDIR"starbox_left.png",
		TEXTUREDIR"starbox_top.png", 
		TEXTUREDIR"starbox_bottom.png",
		TEXTUREDIR"starbox_front.png", 
		TEXTUREDIR"starbox_back.png",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	skyboxTransform = Matrix4::Translation(Vector3(0, 0, 0));
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
	skyboxTransform = Matrix4::Rotation(1, Vector3(0,0,1));
	viewMatrix = camera->BuildViewMatrix(); 
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	root->Update(dt);
}

void Renderer::RenderScene()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkyBox();
	DrawHeightMap();
	for (auto i = root->GetChildIteratorStart(); i != root->GetChildIteratorEnd(); i++)
		DrawNode(*i);
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
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader->GetProgram(), "modelMatrix"), 1, false, skyboxTransform.values);
	quad->Draw();
	glDepthMask(GL_TRUE);
}

void Renderer::DrawNode(SceneNode* n)
{
	if (n->GetMesh())
	{
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		BindShader(rockShader);

		


		glUniform4fv(glGetUniformLocation(rockShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());

		currentTexture = n->GetTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, currentTexture);

		glUniform1i(glGetUniformLocation(rockShader->GetProgram(), "useTexture"), currentTexture);
		UpdateShaderMatrices();
		//readjust model matrix, so it matches the values we actually want, though.
		glUniformMatrix4fv(glGetUniformLocation(rockShader->GetProgram(), "modelMatrix"), 1, false, model.values);
		n->Draw(*this);
	}
}