#include "Renderer.h"
#include "../nclgl/Light.h"

#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include <algorithm>

#define SHADOWSIZE 2048
constexpr int POST_PASSES = 10;

Renderer::Renderer(Window& parent)
	:OGLRenderer(parent)
{
	quad = Mesh::GenerateQuad();
	iceQuad = Mesh::GenerateQuad();
	iceQuad->GenerateNormals();
	iceQuad->GenerateTangents();

	LoadTerrain();	
	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	camera = new Camera(-8, 120.0f, 0, heightmapSize * Vector3(0.8f, 0.5f, 0.34f));
	pointToSun = Vector3(0, 1, 0);
	pointToSatellite = Vector3(0, 1, 0);
	spotlight = new Spotlight(camera->GetPosition(), Vector4(0.8,0.875,1, 1), 5000, 40);
	sunLight = new DirectionLight(pointToSun, Vector4(1, 0.75, 0.75, 1));
	lights.push_back(spotlight);
	lights.push_back(sunLight);
	GenerateShadowFBOs();
	LoadPostProcess();

	
	LoadCubeMap();
	root = new SceneNode();
	
	PlaceMesh(600 * 16, 660 * 16, rockAlbedo, rockBump, rockShader, "rock_01.msh");	
	PlaceMesh(601 * 16, 659 * 16, rockAlbedo, rockBump, rockShader, "rock_01.msh");
	PlaceMesh(646 * 16, 500 * 16, rockAlbedo, rockBump, rockShader, "rock_01.msh");
	PlaceMesh(220 * 16, 506 * 16, rockAlbedo, rockBump, rockShader, "rock_01.msh");
	PlaceMesh(264 * 16, 610 * 16, rockAlbedo, rockBump, rockShader, "rock_01.msh");
	PlaceMesh(287 * 16, 610 * 16, rockAlbedo, rockBump, rockShader, "rock_01.msh");
	PlaceMesh(610 * 16, 540 * 16, rockAlbedo, rockBump, rockShader, "rock_01.msh");
	PlaceMesh(499 * 16, 508 * 16, rockAlbedo, rockBump, rockShader, "rock_01.msh");
	PlaceMesh(447 * 16, 549 * 16, rockAlbedo, rockBump, rockShader, "rock_01.msh");
	PlaceMesh(811 * 16, 414 * 16, rockAlbedo, rockBump, rockShader, "rock_01.msh");
	PlaceMesh(410 * 16, 651 * 16, rockAlbedo, rockBump, rockShader, "rock_01.msh");
	PlaceMesh(309 * 16, 509 * 16, rockAlbedo, rockBump, rockShader, "rock_01.msh");
	

	Shader* sunShader = new Shader("sunVert.glsl", "sunFrag.glsl");
	if (!sunShader->LoadSuccess()) return;

	sun = new SceneNode();
	GLuint sunTex = SOIL_load_OGL_texture(TEXTUREDIR"2K_sun.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	SetTextureRepeating(sunTex, true);
	sun->SetAlbedoTexture(sunTex);
	sun->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	sun->SetModelScale(Vector3(500.0f, 500.0f, 500.0f));
	sun->SetShader(sunShader);	
	sun->SetMesh(Mesh::LoadFromMeshFile("Sphere.msh"));
	sun->SetTransform(Matrix4::Translation(camera->GetPosition() + pointToSun * 50000) * Matrix4::Rotation(90, Vector3(1, 0, 0)));
	root->AddChild(sun);
	
	
	sun->SetTransform(Matrix4::Translation(camera->GetPosition() + pointToSun * 50000) * Matrix4::Rotation(90, Vector3(1,0,0)));

	
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
	delete reflectShader;
	delete spotlight;
	delete quad;
	delete iceQuad;
	delete root;
	glDeleteTextures(1, &heightmapTexSand);
	glDeleteTextures(1, &heightmapBump);
	glDeleteTextures(1, &cubeMap);
	glDeleteTextures(1, &rockAlbedo);
	glDeleteTextures(1, &rockBump);
	glDeleteTextures(1, &iceAlbedo);
	glDeleteTextures(1, &iceBump);
	for (int i = 0; i < 2; i++)
	{
		glDeleteFramebuffers(1, &shMapTex[i].shadowFBO);
		glDeleteTextures(1, &shMapTex[i].shadowTex);
	}
	
}

void Renderer::LoadCubeMap()
{
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"starbox_right.png",
		TEXTUREDIR"starbox_left.png",
		TEXTUREDIR"starbox_top.png",
		TEXTUREDIR"starbox_bottom.png",
		TEXTUREDIR"starbox_front.png",
		TEXTUREDIR"starbox_back.png",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	reflectShader = new Shader("reflectingvert.glsl", "reflectingfrag.glsl");
	if (!skyboxShader->LoadSuccess() || !reflectShader->LoadSuccess() || !cubeMap) return;
}

void Renderer::LoadTerrain()
{
	heightMap = new HeightMap(TEXTUREDIR"heightmapfjord.png", 16, 3, 16);
	heightmapTexSand = SOIL_load_OGL_texture(TEXTUREDIR"TCom_Sand_Muddy2_2x2_512_albedo.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	heightmapBump = SOIL_load_OGL_texture(TEXTUREDIR"TCom_Sand_Muddy2_2x2_512_normal.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	iceAlbedo = SOIL_load_OGL_texture(TEXTUREDIR"Ice_001_COLOR.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	iceBump = SOIL_load_OGL_texture(TEXTUREDIR"Ice_001_NRM.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	rockAlbedo = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	rockBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	lightShader = new Shader("shadowscenevert.glsl", "shadowscenefrag.glsl");
	shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");
	rockShader = new Shader("rockVert.glsl", "rockFrag.glsl");


	if (!lightShader->LoadSuccess() || !shadowShader->LoadSuccess() || !rockShader->LoadSuccess()||!heightmapTexSand || !heightmapBump ||
		!iceAlbedo || !iceBump || !rockAlbedo || !rockBump) return;
	
	SetTextureRepeating(heightmapTexSand, true);
	SetTextureRepeating(heightmapBump, true);
	SetTextureRepeating(iceAlbedo, true);
	SetTextureRepeating(iceBump, true);
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
	ResetViewProjToCamera();
	sceneTime += dt;
	spotlight->SetPosition(Matrix4::Translation(Vector3(0,-100,0))*camera->GetPosition());
	pointToSun = Matrix4::Rotation(dt * 5, Vector3(1,0,0)) * pointToSun;
	pointToSatellite = Matrix4::Rotation(dt * 2, Vector3(1, 0, 0)) * pointToSatellite;
	sunLight->SetDirection(pointToSun);
	sun->SetTransform(Matrix4::Translation(camera->GetPosition() + pointToSun * 50000) * Matrix4::Rotation(90, Vector3(1, 0, 0)));
	//deal with the sun lighting rocks sticking through the ground being 'lit' from below
	horizonCheck = std::min(Vector3::Dot(pointToSun, Vector3(0, 1, 0)) * 10, 1.0f);
	if (horizonCheck < 0) horizonCheck = 0;
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
	DrawShadowScene();
	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	DrawSkyBox();
	DrawHeightMap();
	DrawIce();	
	for (const auto& i : nodeList) DrawNode(i);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	DrawSinglePassPostProcess();
	PresentScene();
	
	//don't forget to clear them for the next frame!
	nodeList.clear();
}

void Renderer::DrawHeightMap()
{
	BindShader(lightShader);
	SetShaderSpotlight(*spotlight);
	SetShaderDirectionLight(*sunLight);

	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmapTexSand);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightmapBump);
	glUniform1f(glGetUniformLocation(lightShader->GetProgram(), "dirHorizonCheck"), horizonCheck);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());	
	
	for (char i = 0; i < lights.size(); i++)
	{
		char name[11] = { "shadowTexN"};
		name[9] = i + 49;

		glUniform1i(glGetUniformLocation(lightShader->GetProgram(), name), i + 2);
		glActiveTexture(GL_TEXTURE2 + i);
		glBindTexture(GL_TEXTURE_2D, shMapTex[i].shadowTex);
	}
	UpdateShaderMatrices();
	//shadow matrices are stored in shMapTex instead of on OGL renderer, so need to add them here
	glUniformMatrix4fv(glGetUniformLocation(lightShader->GetProgram(), "shadowMatrix1"), 1, false, shMapTex[0].shadowMatrix.values);
	glUniformMatrix4fv(glGetUniformLocation(lightShader->GetProgram(), "shadowMatrix2"), 1, false, shMapTex[1].shadowMatrix.values);
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
		SetShaderSpotlight(*spotlight);
		SetShaderDirectionLight(*sunLight);
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "diffuseTex"), 0);
		glUniform3fv(glGetUniformLocation(n->GetShader()->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
		glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "bumpTex"), 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, n->GetAlbedoTexture());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, n->GetBumpTexture());
		UpdateShaderMatrices();
		
		glUniform1f(glGetUniformLocation(lightShader->GetProgram(), "dirHorizonCheck"), horizonCheck);
		//shadow matrices are stored in shMapTex instead of on OGL renderer, so need to add them here
		//glUniformMatrix4fv(glGetUniformLocation(n->GetShader()->GetProgram(), "shadowMatrix1"), 1, false, shMapTex[0].shadowMatrix.values);
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
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	for (int i = 0; i < lights.size(); i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, shMapTex[i].shadowFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
		//turn off colours for the shadow pass
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		BindShader(shadowShader);

		if (lights[i]->GetName() == "spot")
		{
			viewMatrix = Matrix4::BuildViewMatrixFromNormal(spotlight->GetPosition(), spotlight->GetDirection(), Vector3(0, 1, 0));
			projMatrix = Matrix4::Perspective(100, 5000, 1, 60);
		}
		else
		{
			viewMatrix = Matrix4::BuildViewMatrixFromNormal(Vector3(0.5, 0, 0.5) * heightMap->GetHeightmapSize(), -pointToSun, Vector3(0, 0, -1));
			projMatrix = Matrix4::Orthographic(-heightMap->GetHeightmapSize().x, heightMap->GetHeightmapSize().x,
				heightMap->GetHeightmapSize().x / 2, -heightMap->GetHeightmapSize().x / 2,
				heightMap->GetHeightmapSize().z / 2, -heightMap->GetHeightmapSize().z / 2);
		}
		shMapTex[i].shadowMatrix = projMatrix * viewMatrix;
		
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
	glDisable(GL_CULL_FACE);
	ResetViewProjToCamera();
}

void Renderer::PlaceMesh(float x, float z, GLuint albedo, GLuint bump, Shader* shader, std::string meshName)
{
	Vector3 translation;
	if (meshName == "rock_base_LP.msh") translation = Vector3(x, heightMap->GetHeightAtXZ(x, z) - 1850, z);
	if(meshName == "rock_01.msh") translation = Vector3(x, heightMap->GetHeightAtXZ(x, z), z);
	SceneNode* s = new SceneNode();
	s->SetAlbedoTexture(albedo);
	s->SetBumpTexture(bump);
	s->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	s->SetTransform(Matrix4::Translation(translation));
	s->SetModelScale(Vector3(1.0f, 1.0f, 1.0f));
	s->SetShader(shader);
	s->SetMesh(Mesh::LoadFromMeshFile(meshName));
	root->AddChild(s);
}

void Renderer::ResetViewProjToCamera()
{	
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 150000.0f, (float)width / (float)height, 45.0f);
}

void Renderer::DrawIce()
{
	BindShader(reflectShader);
	SetShaderSpotlight(*spotlight);
	SetShaderDirectionLight(*sunLight);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, iceAlbedo);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, iceBump);


	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 1);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "bumpTex"), 2);

	

	Vector3 hSize = heightMap->GetHeightmapSize();
	modelMatrix = Matrix4::Translation(Vector3(hSize.x/2, hSize.y * 0.18, hSize.z/2)) * Matrix4::Scale(hSize * 0.5) * Matrix4::Rotation(90, Vector3(1, 0, 0));

	UpdateShaderMatrices();
	iceQuad->Draw();
}

void Renderer::LoadPostProcess()
{
	pProcOutShader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");
	pProcShader = new Shader("TexturedVertex.glsl", "filmgrainFrag.glsl");

	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	for (int i = 0; i < 2; i++)
	{
		glGenTextures(1, &bufferColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &processFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColourTex[0])
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawMultiPassPostProcess()
{
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(pProcShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(pProcShader->GetProgram(), "sceneTex"), 0);

	for (int i = 0; i < POST_PASSES; i++)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
		glUniform1i(glGetUniformLocation(pProcShader->GetProgram(), "isVertical"), 0);

		glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
		quad->Draw();

		//swap the colour buffers
		glUniform1i(glGetUniformLocation(pProcShader->GetProgram(), "isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);
		quad->Draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::DrawSinglePassPostProcess()
{
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
	BindShader(pProcShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	glUniform1f(glGetUniformLocation(pProcShader->GetProgram(), "sceneTime"), sceneTime);
	float signalLoss = std::min(std::max(1 - abs(Vector3::Dot(pointToSatellite,Vector3(0,1,0))), 0.0f), 0.65f);
	glUniform1f(glGetUniformLocation(pProcShader->GetProgram(), "signalLoss"), signalLoss);
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);	
	glUniform1i(glGetUniformLocation(pProcShader->GetProgram(), "sceneTex"), 0);
	quad->Draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::PresentScene()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(pProcOutShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
	glUniform1i(glGetUniformLocation(pProcOutShader->GetProgram(), "diffuseTex"), 0);
	quad->Draw();
}