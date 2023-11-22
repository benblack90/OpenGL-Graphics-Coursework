#include "SolarSystem.h"

SolarSystem::SolarSystem(SceneNode* sun,  Shader* shader, GLuint planetTex, GLuint gasTex)
{
	pointToPlanet = { 0,0,1 };
	pointToSatellite = { 0.9446, 0.266423, 0.191744 };
	pointToGasGiant = { 1,0,0};
	this->AddChild(sun);
	this->sun = sun;
	mainPlanetTex = planetTex;
	this->shader = shader;
	gasGiantTex = gasTex;
	
}

void SolarSystem::Update(float dt)
{
	
	pointToPlanet = Matrix4::Rotation(dt * 0.01, Vector3(0, 1, 0)) * pointToPlanet;
	pointToSatellite = Matrix4::Rotation(dt * 5, Vector3(0.9446, 0, 0)) * pointToSatellite;
	pointToGasGiant = Matrix4::Rotation(dt * 0.11, Vector3(0, 1, 0)) * pointToGasGiant;
	Matrix4 rotation = Matrix4::Rotation(dt * 10, Vector3(0, 1, 0))* mainPlanet->GetWorldTransform();
	Matrix4 translation = Matrix4::Translation(pointToPlanet * 45000);
	Matrix4 rotlation = rotation;
	rotlation.values[12] = translation.values[12];
	rotlation.values[13] = translation.values[13];
	rotlation.values[14] = translation.values[14];
	mainPlanet->SetTransform(rotlation);
	satellite->SetTransform(Matrix4::Translation(pointToSatellite * 250));
	gasGiant->SetTransform(Matrix4::Translation(pointToGasGiant * 35000));

	SceneNode::Update(dt);
}

void SolarSystem::Active()
{

	mainPlanet = new SceneNode();

	mainPlanet->SetAlbedoTexture(mainPlanetTex);
	mainPlanet->SetColour({ 1,1,1,1 });
	mainPlanet->SetModelScale({ 200,200,200 });
	mainPlanet->SetShader(shader);
	mainPlanet->SetMesh(Mesh::LoadFromMeshFile("Sphere.msh"));
	mainPlanet->SetTransform(Matrix4::Translation(pointToPlanet * 45000));
	sun->AddChild(mainPlanet);

	satellite = new SceneNode();
	satellite->SetAlbedoTexture(SOIL_load_OGL_texture(TEXTUREDIR"TCom_SolarCells_512_albedo.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	satellite->SetBumpTexture(SOIL_load_OGL_texture(TEXTUREDIR"TCom_SolarCells_512_normal.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));
	satellite->SetColour({ 1,1,1,1 });
	satellite->SetShader(shader);
	satellite->SetMesh(Mesh::LoadFromMeshFile("Cube.msh"));
	satellite->SetTransform(mainPlanet->GetWorldTransform());
	mainPlanet->AddChild(satellite);

	gasGiant = new SceneNode();
	gasGiant->SetAlbedoTexture(gasGiantTex);
	gasGiant->SetColour({ 1,1,1,1 });
	gasGiant->SetModelScale({ 350,350,350 });
	gasGiant->SetShader(shader);
	gasGiant->SetMesh(Mesh::LoadFromMeshFile("Sphere.msh"));
	gasGiant->SetTransform(Matrix4::Translation(pointToGasGiant * 35000));
	sun->AddChild(gasGiant);
}
void SolarSystem::Inactive()
{
	sun->RemoveChild(mainPlanet);
}