#include "Renderer.h"
#include "../nclgl/Light.h"

#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include <algorithm>

#define SHADOWSIZE 4096
constexpr int POST_PASSES = 8;

Renderer::Renderer(Window& parent)
	:OGLRenderer(parent)
{
	planetSide = true;
	autoCamera = true;
	toneMapping = true;
	showHud = true;
	showNoise = true;
	quad = Mesh::GenerateQuad();

	LoadTerrain();	
	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	camera = new Camera(-30, 120.0f, 0, heightmapSize * Vector3(0.8f, 0.75f, 0.34f));
	pointToSun = Vector3(0, 1, 0);
	pointToSatellite = Vector3(0, 1, 0);
	spotlight = new Spotlight(camera->GetPosition(), Vector4(1.25,1.02,1.5,1), 5000, 40);
	sunLight = new DirectionLight(pointToSun, Vector4(1, 0.75, 0.75, 1));
	lights.push_back(spotlight);
	lights.push_back(sunLight);
	GenerateShadowFBOs();
	LoadPostProcess();	
	LoadCubeMap();
	LoadHudTexes();
	
	planetRoot = new SceneNode();
	LoadSun();
	GLuint planetTex = SOIL_load_OGL_texture(TEXTUREDIR"2k_ceres_fictional.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	SetTextureRepeating(planetTex,true);
	GLuint gasTex = SOIL_load_OGL_texture(TEXTUREDIR"2k_uranus.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	SetTextureRepeating(gasTex, true);
	orbitRoot = new SolarSystem(sun, lightShader, planetTex, gasTex);
	
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
	
	
	
	projMatrix = Matrix4::Perspective(1.0f, 150000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	sceneTime = 0.0f;
	hudTimer = 0.0f;
	warnText = false;
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
	delete planetRoot;
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

void Renderer::CheckSceneControlKeys()
{
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_C)) 
		autoCamera = !autoCamera;
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_H))
		showHud = !showHud;
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_T))
		toneMapping = !toneMapping;
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_N))
		showNoise = !showNoise;
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_B))
		showBloom = !showBloom;
	if (!planetSide && (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1) || (camera->GetCameraTimer() > 15) && autoCamera))
	{
		planetSide = true;
		camera->SetCameraTimer(0.0f);
		orbitRoot->Inactive();
		camera->SetPosition(heightMap->GetHeightmapSize() * Vector3(0.8f, 0.75f, 0.34f));
		camera->SetPitch(-30);
		camera->SetYaw(120);
		spotlight->SetRadius(5000);
		//OTHER INTERMEDIARY STEP CODE GOES HERE
	}
	if (planetSide && (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2) || (camera->GetCameraTimer() > 63) && autoCamera))
	{
		planetSide = false;
		camera->SetCameraTimer(0.0f);
		orbitRoot->Active();
		sun->SetTransform(Matrix4::Translation(Vector3(0, 0, 0)));
		orbitRoot->Update(deltaTime);
		camera->SetPosition(orbitRoot->GetPlanetPosition().GetPositionVector() + Vector3(-550, 0, 0));
		camera->SetYaw(270);
		camera->SetPitch(0);
		sunLight->SetDirection(orbitRoot->GetPointToPlanet());
		spotlight->SetPosition(Vector3(0, 550, 0));
		spotlight->SetRadius(50000);
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

void Renderer::LoadHudTexes()
{
	fullSigTex = SOIL_load_OGL_texture(TEXTUREDIR"fullSignal.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	hiMidSigTex = SOIL_load_OGL_texture(TEXTUREDIR"midHiSig.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	loMidSigTex = SOIL_load_OGL_texture(TEXTUREDIR"midLoSig.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	loSigWarnTexes[true] = SOIL_load_OGL_texture(TEXTUREDIR"signalLoWarn.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	loSigWarnTexes[false] = SOIL_load_OGL_texture(TEXTUREDIR"signalLoNoWarn.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);	

	if (!fullSigTex || !hiMidSigTex || !loMidSigTex || !loSigWarnTexes[true] || !loSigWarnTexes[false]) return;
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

void Renderer::LoadSun()
{
	sun = new SceneNode();
	GLuint sunTex = SOIL_load_OGL_texture(TEXTUREDIR"2K_sun.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	Shader* sunShader = new Shader("sunVert.glsl", "sunFrag.glsl");
	if (!sunShader->LoadSuccess()) return;
	SetTextureRepeating(sunTex, true);
	sun->SetAlbedoTexture(sunTex);
	sun->SetColour({100,100,100,1});
	sun->SetModelScale({500,500,500});
	sun->SetShader(sunShader);
	sun->SetMesh(Mesh::LoadFromMeshFile("Sphere.msh"));
	sun->SetTransform(Matrix4::Translation(camera->GetPosition() + pointToSun * 50000) * Matrix4::Rotation(90, Vector3(1, 0, 0)));
	planetRoot->AddChild(sun);
}


void Renderer::UpdateScene(float dt)
{
	deltaTime = dt;
	sceneTime += dt;
	CheckSceneControlKeys();	
	camera->UpdateCamera(heightMap, planetSide, autoCamera, dt);
	ResetViewProjToCamera();
	Matrix4 yaw;
	Matrix4 pitch;
	Vector3 forward;
	
	switch (planetSide)
	{
	case true:
		spotlight->SetPosition(Matrix4::Translation(Vector3(0, -100, 0)) * camera->GetPosition());
		pointToSun = Matrix4::Rotation(dt * 5, Vector3(1, 0, 0)) * pointToSun;
		pointToSatellite = Matrix4::Rotation(dt * 6, Vector3(1, 0, 0)) * pointToSatellite;
		sunLight->SetDirection(pointToSun);
		sun->SetTransform(Matrix4::Translation(camera->GetPosition() + pointToSun * 30000) * Matrix4::Rotation(90, Vector3(1, 0, 0)));
		//deal with the sun lighting rocks sticking through the ground being 'lit' from below
		horizonCheck = std::min(Vector3::Dot(pointToSun, Vector3(0, 1, 0)) * 10, 1.0f);
		if (horizonCheck < 0) horizonCheck = 0;
		yaw = Matrix4::Rotation(camera->GetYaw() + (sinf(sceneTime) * 20), Vector3(0, 1, 0));
		pitch = Matrix4::Rotation(camera->GetPitch() - 10, Vector3(1, 0, 0));
		forward = (yaw * pitch * Vector3(0, 0, -1));
		spotlight->SetDirection(forward);
		frameFrustum.FromMatrix(projMatrix * viewMatrix);
		planetRoot->Update(dt);
		break;
	case false:
		frameFrustum.FromMatrix(projMatrix * viewMatrix);
		orbitRoot->Update(dt);
		sunLight->SetDirection(orbitRoot->GetPointToPlanet());
		spotlight->SetDirection(orbitRoot->GetPointToPlanet());
		horizonCheck = 1;
		break;
	}
	
}

void Renderer::RenderScene()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	switch (planetSide)
	{
	case true:
		//rebuild and sort, before drawing
		BuildNodeLists(planetRoot);
		std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
		DrawShadowScene();
		glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		DrawSkyBox();
		DrawHeightMap();
		DrawIce();
		for (const auto& i : nodeList) DrawNode(i);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		DrawMultiPassBlur();
		if(toneMapping)
			HDRToneMap();	
		if(showBloom)
			DrawBloom();
		if(showNoise)
			DrawFilmGrainPass();
		PresentScene();
		if(showHud)
			DrawHud(deltaTime);

		
		break;
	case false:
		glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		BuildNodeLists(orbitRoot);
		std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
		DrawSkyBox();
		for (const auto& i : nodeList) DrawNode(i);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
		DrawMultiPassBlur();
		if (toneMapping)
			HDRToneMap();
		if (showBloom)
			DrawBloom();
		PresentScene();
		break;
	}

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
		
		glUniform1f(glGetUniformLocation(n->GetShader()->GetProgram(), "dirHorizonCheck"), horizonCheck);
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
	planetRoot->AddChild(s);
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
	quad->Draw();
}

void Renderer::LoadPostProcess()
{
	pProcOutShader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");
	pProcGrainShader = new Shader("TexturedVertex.glsl", "filmgrainFrag.glsl");
	pProcHDRShader = new Shader("basicTextureVert.glsl", "HDRToneFrag.glsl");
	pProcBloomExtract = new Shader("basicTextureVert.glsl", "bloomExtractFrag.glsl");
	pProcBlurShader = new Shader("TexturedVertex.glsl", "processfrag.glsl");
	if (!pProcOutShader->LoadSuccess() || !pProcGrainShader->LoadSuccess() || !pProcHDRShader->LoadSuccess() 
		|| !pProcBloomExtract->LoadSuccess() || !pProcBlurShader->LoadSuccess() ) return;

	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	for (int i = 0; i < 2; i++)
	{
		glGenTextures(1, &blurColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, blurColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	}

	glGenTextures(1, &bufferTex);
	glBindTexture(GL_TEXTURE_2D, bufferTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenFramebuffers(1, &bufferFBO);
	glGenFramebuffers(1, &processFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferTex, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferTex)
		return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawMultiPassBlur()
{
	
	//make the bloom map
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurColourTex[0], 0);
	BindShader(pProcBloomExtract);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferTex);
	quad->Draw();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurColourTex[1], 0);
	quad->Draw();
	BindShader(pProcBlurShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();
	glUniform1i(glGetUniformLocation(pProcBlurShader->GetProgram(), "sceneTex"), 0);
	for (int i = 0; i < POST_PASSES; i++)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurColourTex[1], 0);
		glUniform1i(glGetUniformLocation(pProcBlurShader->GetProgram(), "isVertical"), 0);
		
		glBindTexture(GL_TEXTURE_2D, blurColourTex[0]);
		quad->Draw();

		//swap the colour buffers
		glUniform1i(glGetUniformLocation(pProcBlurShader->GetProgram(), "isVertical"), 1);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurColourTex[0], 0);
		glBindTexture(GL_TEXTURE_2D, blurColourTex[1]);
		quad->Draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	
}

void Renderer::HDRToneMap()
{

	//make the HDR tonemap
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferTex, 0);
	BindShader(pProcHDRShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferTex);
	glUniform1i(glGetUniformLocation(pProcHDRShader->GetProgram(), "hiTex"), 0);
	quad->Draw();

	

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawBloom()
{
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferTex, 0);
	glBindTexture(GL_TEXTURE_2D, blurColourTex[0]);
	BindShader(pProcOutShader);
	glActiveTexture(GL_TEXTURE0);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();
	glUniform1i(glGetUniformLocation(pProcOutShader->GetProgram(), "diffuseTex"), 0);
	glBlendFunc(GL_ONE, GL_ONE);
	quad->Draw();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawFilmGrainPass()
{
	glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferTex, 0);
	BindShader(pProcGrainShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	glUniform1f(glGetUniformLocation(pProcGrainShader->GetProgram(), "sceneTime"), sceneTime);
	//signal loss ranges from 0 to 0.65, when the satellite is on the opposite side of the planet
	signalLoss = abs(std::min(0.35f + Vector3::Dot(pointToSatellite, Vector3(0, 1, 0)), 0.0f));
	glUniform1f(glGetUniformLocation(pProcGrainShader->GetProgram(), "signalLoss"), signalLoss);
	UpdateShaderMatrices();

	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, bufferTex);	
	glUniform1i(glGetUniformLocation(pProcGrainShader->GetProgram(), "sceneTex"), 0);
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
	glBindTexture(GL_TEXTURE_2D, bufferTex);
	glUniform1i(glGetUniformLocation(pProcOutShader->GetProgram(), "diffuseTex"), 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	quad->Draw();
}

void Renderer::DrawHud(float dt)
{
	glDisable(GL_DEPTH_TEST);
	
	modelMatrix = Matrix4::Rotation(180, Vector3(0, 1, 0)) * Matrix4::Rotation(180, Vector3(0, 0, -1));
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	if(signalLoss <= 0.0f) glBindTexture(GL_TEXTURE_2D, fullSigTex);
	if (signalLoss > 0.0f) glBindTexture(GL_TEXTURE_2D, hiMidSigTex);
	if (signalLoss > 0.25f) glBindTexture(GL_TEXTURE_2D, loMidSigTex);
	if (signalLoss > 0.45f)
	{
		hudTimer += dt;
		//flash text on/text off version every second
		glBindTexture(GL_TEXTURE_2D, loSigWarnTexes[warnText]);
		if (hudTimer > 1)
		{
			hudTimer = 0.0f;
			warnText = !warnText;
		}

	}
	UpdateShaderMatrices();
	quad->Draw();
	glEnable(GL_DEPTH_TEST);
}