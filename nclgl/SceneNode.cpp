#include "SceneNode.h"

SceneNode::SceneNode(Mesh* mesh, Vector4 colour)
{
	this->mesh = mesh;
	if (mesh) boundingRadius = mesh->GenerateBoundingValues();
	this->colour = colour;
	parent = nullptr;
	modelScale = Vector3(1, 1, 1);
	distanceFromCamera = 0.0f;
	albedoTex = 0;
	bumpTex = 0;
}

//NOTE how this destructor does not destroy its mesh. This is because multiple scene nodes may point to the same mesh, and we don't want to delete the mesh when 
//one SceneNode is deleted!
SceneNode::~SceneNode(void)
{
	for (unsigned int i = 0; i < children.size(); i++)
	{
		delete children[i];
	}
}

void SceneNode::AddChild(SceneNode* s)
{
	//can't be a parent of yourself!
	if (s == this)
	{
		std::cout << "ERROR: Failed to add child, because pointer passed was pointer to self" << '\n';
		return;
	}
	if (s->mesh) 
		s->boundingRadius = s->mesh->GenerateBoundingValues();
	children.push_back(s);
	s->parent = this;
}

void SceneNode::RemoveChild(SceneNode* s)
{
	auto iter = std::find(children.begin(), children.end(), s);
	if (iter != children.end())
	{
		delete *iter;
		children.erase(iter);
	}
}

void SceneNode::Draw(const OGLRenderer &r)
{
	if (mesh) mesh->Draw();
}

void SceneNode::Update(float dt)
{
	if (parent)
	{
		worldTransform = parent->worldTransform * transform;
	}
	else
	{
		worldTransform = transform;
	}
	for (vector<SceneNode*>::iterator i = children.begin(); i != children.end(); i++)
	{
		(*i)->Update(dt);
	}
}