#include "Camera.h"
#include "Window.h"
#include <algorithm>

void Camera::UpdateCamera(float dt)
{
	Matrix4 rotation	= Matrix4::Rotation(yaw, Vector3(0, 1, 0)); //(0, 1, 0) around the y axis
	Vector3 forward		= rotation * Vector3(0, 0, -1);
	Vector3 right		= rotation * Vector3(1, 0, 0);

	if (isAuto)
	{
		timePice += dt;

		if (yaw < 0)
		{
			yaw += 360.0f;
		}
		if (yaw > 360.0f)
		{
			yaw -= 360.0f;
		}
		pitch = std::min(pitch, 90.0f);
		pitch = std::max(pitch, -90.0f);
		float speed = 0.6f;
		if (timePice < 10)
		{
			position	+= forward * speed;
			position	+= right * speed;
		}
		if (timePice >= 10 && timePice < 20)
		{
			position.y	-= speed;
			position	-= right * speed;
		}
		if (timePice >= 20 && timePice < 30)
		{
			pitch -= 10 * dt;
		}
		if (timePice >= 30 && timePice < 40)
		{
			position -= forward * speed;
			position.y += speed;
		}
		if (timePice >= 40 && timePice < 50)
		{
			yaw += 10 * dt;
		}
		if (timePice >= 50 && timePice < 60)
		{
			position += forward * speed * 10;
		}
		if (timePice >= 60 && timePice < 70)
		{
			yaw -= 30 * dt;
		}

		float rotateSpeed	= 60.0f * dt;
		Vector3 forward		= Matrix4::Rotation(dt * 300, Vector3(0, 1, 0)) * Vector3(0, 0, -1);
		position			+= forward * rotateSpeed;
	}
	else
	{
		pitch	-= Window::GetMouse()->GetRelativePosition().y;
		yaw		-= Window::GetMouse()->GetRelativePosition().x;

		pitch	= std::min(pitch, 90.0f);
		pitch	= std::max(pitch, -90.0f);

		if (yaw < 0)
		{
			yaw += 360.0f;
		}
		if (yaw > 360.0f)
		{
			yaw -= 360.0f;
		}

		float speed = 1000.0f * dt;

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
	}
}

void Camera::UpdateMapCamera(Camera* mainCam, float dt)
{
	position.x	= mainCam->position.x;
	position.z	= mainCam->position.y;
	yaw			= mainCam->GetYaw();
}

Matrix4 Camera::BuildViewMatrix()
{
	return	Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) *
			Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) *
			Matrix4::Translation(-position);
}