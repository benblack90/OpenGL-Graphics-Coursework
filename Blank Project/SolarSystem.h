#pragma once
#include "../nclgl/SceneNode.h"
#include "../nclgl/OGLRenderer.h"

class SolarSystem : public SceneNode
{
public:
	SolarSystem(SceneNode* sun, Shader* shader, GLuint planetTex, GLuint gasTex);
	~SolarSystem() {};
	void Update(float dt) override;
	Matrix4 GetPlanetPosition() { return mainPlanet->GetWorldTransform(); }
	Vector3 GetPointToPlanet() { return pointToPlanet; }
	void Active();
	void Inactive();

protected:
	SceneNode* sun;
	SceneNode* mainPlanet;
	SceneNode* satellite;
	SceneNode* gasGiant;
	Shader* shader;
	GLuint mainPlanetTex;
	GLuint gasGiantTex;
	GLuint satTex;
	Vector3 pointToPlanet;
	Vector3 pointToSatellite;
	Vector3 pointToGasGiant;
};