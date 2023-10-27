#include "CubeRobot.h"

CubeRobot::CubeRobot(Mesh* cube)
{
	//SetMesh(cube); //Uncomment if you want a local origin marker

	SceneNode* body = new SceneNode(cube, Vector4(1, 0, 0, 1));		//Red body
	body->SetModelScale(Vector3(10, 15, 5));
	body->SetTransform(Matrix4::Translation(Vector3(0, 35, 0)));
	AddChild(body);

	head = new SceneNode(cube, Vector4(0, 1, 0, 1));				//Green head
	head->SetModelScale(Vector3(5, 5, 5));
	head->SetTransform(Matrix4::Translation(Vector3(0, 30, 0)));
	body->AddChild(head);

	leftArm = new SceneNode(cube, Vector4(0, 0, 1, 1));				//Blue arms
	leftArm->SetModelScale(Vector3(3, -18, 3));
	leftArm->SetTransform(Matrix4::Translation(Vector3(-12, 30, -1)));
	body->AddChild(leftArm);

	rightArm = new SceneNode(cube, Vector4(0, 0, 1, 1));			//Blue again
	rightArm->SetModelScale(Vector3(3, -18, 3));
	rightArm->SetTransform(Matrix4::Translation(Vector3(12, 30, -1)));
	body->AddChild(rightArm);

	SceneNode* hips = new SceneNode();
	body->AddChild(hips);

	SceneNode* leftLeg = new SceneNode(cube, Vector4(0, 0, 1, 1));	//Blue legz hell yeah
	leftLeg->SetModelScale(Vector3(3, -17.5, 3));
	leftLeg->SetTransform(Matrix4::Translation(Vector3(-8, 0, 0)));
	hips->AddChild(leftLeg);

	SceneNode* rightLeg = new SceneNode(cube, Vector4(0, 0, 1, 1));	//Blue leg 2
	rightLeg->SetModelScale(Vector3(3, -17.5, 3));
	rightLeg->SetTransform(Matrix4::Translation(Vector3(8, 0, 0)));
	hips->AddChild(rightLeg);

	body->SetBoundingRadius(15.0f);
	head->SetBoundingRadius(5.0f);

	leftArm->SetBoundingRadius(18.0f);
	rightArm->SetBoundingRadius(18.0f);

	leftLeg->SetBoundingRadius(18.0f);
	rightLeg->SetBoundingRadius(18.0f);
}

void CubeRobot::Update(float dt)
{
	//spin entire robot
	transform = transform * Matrix4::Rotation(30.0f * dt, Vector3(0, 1, 0));

	//spin head
	head->SetTransform(head->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(0, 1, 0)));

	//weeeeeee
	leftArm->SetTransform(leftArm->GetTransform() * Matrix4::Rotation(-30.0f * dt, Vector3(1, 0, 0)));
	rightArm->SetTransform(rightArm->GetTransform() * Matrix4::Rotation(30.0f * dt, Vector3(1, 0, 0)));

	SceneNode::Update(dt);
}