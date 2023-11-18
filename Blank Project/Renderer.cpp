#include "Renderer.h"
#include "../nclgl/Light.h"

#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include <algorithm>

#define SHADOWSIZE 2048

Renderer::Renderer(Window& parent)
	:OGLRenderer(parent)
{
	quad = Mesh::GenerateQuad();
	LoadTerrain();	
	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	camera = new Camera(-15.0f, 0.0f, 0, heightmapSize * Vector3(0.5f, 1.0f, 0.5f));

	spotlight = new Spotlight(camera->GetPosition(), Vector4(1, 1, 1, 1), 5000, 40);
	sunLight = new DirectionLight(Vector3(0, -1, 0), Vector4(1, 1, 1, 1));
	lights.push_back(spotlight);
	lights.push_back(sunLight);
	GenerateShadowFBOs();

	root = new SceneNode();
	SceneNode* s = new SceneNode();
	s->SetAlbedoTexture(SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	s->SetBumpTexture(SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	s->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	s->SetTransform(Matrix4::Translation(heightmapSize * Vector3(0.45f, 0.60f, 0.45f)));
	s->SetModelScale(Vector3(1.0f, 1.0f, 1.0f));
	s->SetShader(lightShader);
	s->SetMesh(Mesh::LoadFromMeshFile("rock_02.msh"));
	root->AddChild(s);

	Shader* sunShader = new Shader("sunVert.glsl", "sunFrag.glsl");
	if (!sunShader->LoadSuccess()) return;
	sun = new SceneNode();
	sun->SetAlbedoTexture(SOIL_load_OGL_texture(TEXTUREDIR"2K_sun.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	sun->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	sun->SetModelScale(Vector3(500.0f, 500.0f, 500.0f));
	sun->SetShader(sunShader);	
	sun->SetMesh(Mesh::LoadFromMeshFile("Sphere.msh"));
	root->AddChild(sun);

	pointToSun = Vector3(0, 1, 0);
	sun->SetTransform(Matrix4::Translation(camera->GetPosition() + pointToSun * 50000) * Matrix4::Rotation(90, Vector3(1,0,0)));
	sunlightOrigin = Vector3(0.5, 0, 0.5) * heightMap->GetHeightmapSize() + pointToSun * 50000;

	LoadCubeMap();
	projMatrix = Matrix4::Perspective(1.0f, 150000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	sceneTime = 0.0f;
	init = true;
}

Renderer::~Renderer()
{
	delete camera;
	delete heightMap;
	delete lightShader;
	delete skyboxShader;
	delete spotlight;
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
	if (!skyboxShader->LoadSuccess() || !cubeMap) return;
}

void Renderer::LoadTerrain()
{
	heightMap = new HeightMap(TEXTUREDIR"heightmapfjord.png");
	heightmapTex = SOIL_load_OGL_texture(TEXTUREDIR"TCom_Sand_Muddy2_2x2_512_albedo.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	heightmapBump = SOIL_load_OGL_texture(TEXTUREDIR"TCom_Sand_Muddy2_2x2_512_normal.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	lightShader = new Shader("shadowscenevert.glsl", "shadowscenefrag.glsl");
	shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");
	if (!lightShader->LoadSuccess() || !shadowShader->LoadSuccess() || !heightmapTex || !heightmapBump) return;
	
	SetTextureRepeating(heightmapTex, true);
	SetTextureRepeating(heightmapBump, true);
	
	glEnable(GL_DEPTH_TEST);
}

void Renderer::GenerateShadowFBOs()
{
	for (int i = 0; i < lights.size(); i++)
	{
		glGenTextures(1, &shMapTex[i].shadowTex);
		glBindTexture(GL_TEXTURE_2D, shMapTex[i].shadowTex);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &shMapTex[i].shadowFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, shMapTex[i].shadowFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shMapTex[i].shadowTex, 0);
		glDrawBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}	
}

void Renderer::UpdateScene(float dt)
{
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	sceneTime += dt;
	spotlight->SetPosition(Matrix4::Translation(Vector3(0,-100,0))*camera->GetPosition());
	pointToSun = Matrix4::Rotation(dt * 1, Vector3(1,0,0)) * pointToSun;
	sun->SetTransform(Matrix4::Translation(camera->GetPosition() + pointToSun * 50000) * Matrix4::Rotation(90, Vector3(1, 0, 0)));
	sunlightOrigin = Vector3(0.5, 0, 0.5) * heightMap->GetHeightmapSize() + pointToSun * 50000;

	Matrix4 yaw = Matrix4::Rotation(camera->GetYaw() + (sinf(sceneTime) * 20), Vector3(0, 1, 0));
	Matrix4 pitch = Matrix4::Rotation(camera->GetPitch() - 10, Vector3(1, 0, 0));
	Vector3 forward = (yaw * pitch * Vector3(0, 0, -1));
	spotlight->SetDirection(forward);
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	root->Update(dt);
}

void Renderer::RenderScene()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	//rebuild and sort, before drawing
	BuildNodeLists(root);
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
	DrawSkyBox();
	DrawShadowScene();		
	DrawHeightMap();

	
	for (const auto& i : nodeList) DrawNode(i);
	
	//don't forget to clear them for the next frame!
	nodeList.clear();
}

void Renderer::DrawHeightMap()
{
	BindShader(lightShader);
	SetShaderSpotlight(*spotlight);
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 1500000.0f, (float)width / (float)height, 45.0f);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmapTex);
	
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightmapBump);
	
	for (char i = 0; i < lights.size(); i++)
	{
		char name[11] = { "shadowTexN"};
		name[9] = i + 49;

		glUniform1i(glGetUniformLocation(lightShader->GetProgram(), name), i + 2);
		glActiveTexture(GL_TEXTURE2 + i);
		glBindTexture(GL_TEXTURE_2D, shMapTex[i].shadowTex);
	}
	
	
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

void Renderer::DrawNode(SceneNode* n)
{
	if (n->GetMesh())
	{
		BindShader(n->GetShader());
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, n->GetAlbedoTexture());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, n->GetBumpTexture());
		UpdateShaderMatrices();
		//readjust model matrix, so it matches the values we actually want, though.
		glUniformMatrix4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "modelMatrix"), 1, false, model.values);
		n->Draw(*this);
	}
}

void Renderer::BuildNodeLists(SceneNode* from)
{
	if (frameFrustum.InsideFrustum(*from))
	{
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();

		//dot product of self is magnitude squared
		from->SetCameraDistance(Vector3::Dot(dir, dir));
		nodeList.push_back(from);
	}

	for (auto i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); i++)
	{
		BuildNodeLists((*i));
	}
}

void Renderer::DrawShadowScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, shMapTex[0].shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	//turn off colours for the shadow pass
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	BindShader(shadowShader);

	viewMatrix = Matrix4::BuildViewMatrixFromNormal(spotlight->GetPosition(),spotlight->GetDirection(), Vector3(0,1,0));
	projMatrix = Matrix4::Perspective(1, 15000, 1, 60);
	shadowMatrix = projMatrix * viewMatrix;
	
	//heightMap->Draw();
	for (SceneNode* s : nodeList)
	{
		modelMatrix = s->GetWorldTransform();
		UpdateShaderMatrices();
		s->Draw(*this);
	}
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();

	//turn colours back on!
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

