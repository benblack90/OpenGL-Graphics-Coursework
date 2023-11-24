#include "Camera.h"
#include "Window.h"
#include <algorithm>

void Camera::UpdateCamera(HeightMap* heightMap, bool planetSide, bool autoCamera, float dt)
{

	if (!autoCamera) ReadCamControl(dt);
	if (autoCamera) FollowTrack(dt, planetSide, heightMap);

}

void Camera::UpdateCamera(float dt)
{
	ReadCamControl(dt);
}

void Camera::ReadCamControl(float dt)
{
	pitch -= (Window::GetMouse()->GetRelativePosition().y) * pitchSpeed;
	yaw -= (Window::GetMouse()->GetRelativePosition().x) * yawSpeed;

	pitch = std::min(pitch, 90.0f);
	pitch = std::max(pitch, -90.0f);

	if (yaw < 0)
	{
		yaw += 360.0f;
	}

	if (yaw > 360.0f)
	{
		yaw -= 360.0f;
	}

	rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));
	forward = rotation * Vector3(0, 0, -1);
	right = rotation * Vector3(1, 0, 0);

	float speed = 300.0f * dt;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W))
	{
		position += forward * speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S))
	{
		position -= forward * speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A))
	{
		position -= right * speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D))
	{
		position += right * speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT))
	{
		position.y += speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE))
	{
		position.y -= speed;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_Q))
	{
		roll--;
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_E))
	{
		roll++;
	}
	if (roll < 0)
	{
		roll += 360.0f;
	}

	if (roll > 360.0f)
	{
		roll -= 360.0f;
	}
}

Matrix4 Camera::BuildViewMatrix()
{
	return Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
		Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
		Matrix4::Rotation(-roll, Vector3(0, 0, 1)) *
		Matrix4::Translation(-position);
}

void Camera::FollowTrack(float dt, bool planetSide,HeightMap* heightMap)
{
	cameraTimer += dt;
	if (planetSide)
	{
		
		forward = rotation * Vector3(0, 0, -1);
		right = rotation * Vector3(1, 0, 0);
		rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));
		position += forward * 210.0f * dt;
		right = right * sinf(cameraTimer);
		position += right * 75 * dt;
		yaw += 1 * dt;
		if (pitch < -8) pitch += 1 * dt;
		if (position.y - heightMap->GetHeightAtXZ(position.x, position.z) < 500) position.y += 5 * dt;
		if (position.y - heightMap->GetHeightAtXZ(position.x, position.z) > 1500) position.y -= 5 * dt;
		if (cameraTimer > 56)
		{
			pitch += 8 * dt;
			position += forward * 300 * dt;
		}
		if (position.x < 5000 || position.z < 5000 || position.x > 14000 || position.z > 11000) yaw -= 12 * dt;		
	}
	if (!planetSide)
	{
		forward = rotation * Vector3(0, 0, -1);
		right = rotation * Vector3(1, 0, 0);
		rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));
		position += right * 75 * dt;
		yaw += 8 * dt;

	}
}