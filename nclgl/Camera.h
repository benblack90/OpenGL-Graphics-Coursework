#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "../nclgl/HeightMap.h"

class Camera
{
public:
	Camera(void)
	{
		yaw = 0.0f;
		pitch = 0.0f;
		roll = 0.0f;
	};


	Camera(float pitch, float yaw, float roll, Vector3 position)
	{
		this->pitch = pitch;
		this->yaw = yaw;
		this->roll = roll;
		this->position = position;
	}

	~Camera(void) {	};

	void UpdateCamera(HeightMap* heightMap, bool planetSide, bool autoCamera, float dt = 1.0f);
	void UpdateCamera(float dt);
	Matrix4 BuildViewMatrix();

	Vector3 GetPosition() const { return position; }
	void SetPosition(Vector3 val) { position = val; }

	float GetYaw() const { return yaw; }
	void SetYaw(float y) { yaw = y; }

	float GetPitch() const { return pitch; }
	void SetPitch(float p) { pitch = p; }

	float GetCameraTimer() const{ return cameraTimer; }
	void SetCameraTimer(float t) { cameraTimer = t; }

protected:
	void ReadCamControl(float dt);
	void FollowTrack(float dt, bool planetSide, HeightMap* heightMap);

	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));
	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);

	float cameraTimer = 0;;
	float yaw;
	float pitch;
	float roll;
	float pitchSpeed = 0.4f;
	float yawSpeed = 0.6f;
	Vector3 position; //0,0,0 by default, via the Vector3 constructor
};