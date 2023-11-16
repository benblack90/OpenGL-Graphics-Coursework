#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Mesh.h"
#include <vector>

class SceneNode
{
public:
	SceneNode(Mesh* m = nullptr, Vector4 colour = Vector4(1, 1, 1, 1));
	~SceneNode(void);

	Matrix4 GetWorldTransform() const { return worldTransform; }

	void SetTransform(const Matrix4& matrix) { transform = matrix; }
	const Matrix4& GetTransform() const { return transform; }
	
	Vector4 GetColour() const { return colour; }
	void SetColour(Vector4 c) { colour = c; }

	Vector3 GetModelScale() const { return modelScale; }
	void SetModelScale(Vector3 s) { modelScale = s; }

	Mesh* GetMesh() const { return mesh; }
	void SetMesh(Mesh* m) { mesh = m; }

	float GetBoundingRadius() const { return boundingRadius; }
	void SetBoundingRadius(float f) { boundingRadius = f; }

	float GetCameraDistance() const { return distanceFromCamera; }
	void SetCameraDistance(float f) { distanceFromCamera = f; }

	GLuint GetAlbedoTexture() const { return albedoTex; }
	void SetAlbedoTexture(GLuint tex) { albedoTex = tex; }

	GLuint GetBumpTexture() const { return bumpTex; }
	void SetBumpTexture(GLuint tex) { bumpTex = tex; }

	void AddChild(SceneNode* s);
	void RemoveChild(SceneNode* s);

	virtual void Update(float dt);
	virtual void Draw(const OGLRenderer &r);
	static bool CompareByCameraDistance(SceneNode* a, SceneNode* b) { return (a->distanceFromCamera < b->distanceFromCamera) ? true : false; }
	std::vector<SceneNode*>::const_iterator GetChildIteratorStart() { return children.begin(); }
	std::vector<SceneNode*>::const_iterator GetChildIteratorEnd() { return children.end(); }

protected:

	SceneNode* parent;							//pointer to parent
	Mesh* mesh;									//pointer to mesh
	Matrix4 worldTransform;						//world coordinates matrix
	Matrix4 transform;							//local (to parent) coordinates matrix
	Vector3 modelScale;							//scaling vector, separate from transforms
	Vector4 colour;								//colour vector							
	std::vector<SceneNode*> children;			//vector of all children, readable via the public getter methods for the const iterator
	float distanceFromCamera;					//self explanatory!
	float boundingRadius = 1.0f;				//radius of the sphere bounding the node
	GLuint albedoTex;							//the node's albedo texture
	GLuint bumpTex;								//the node's bump texture
};