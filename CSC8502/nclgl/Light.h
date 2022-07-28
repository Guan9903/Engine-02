#pragma once

#include "Vector4.h"
#include "Vector3.h"

class Light
{
public:
	Light() {}	//Default constructor, we'll be needing this later!

	/*Point Lighting*/
	Light(const Vector3& position, const Vector4& colour, float radius)
	{
		this->position = position;
		this->colour = colour;
		this->radius = radius;
	}

	/*Directional Lighting*/
	Light(const Vector4& colour, Vector3 direction)
	{
		this->position = -direction;
		this->colour = colour;
		this->direction = direction;
	}

	/*Spot Lighting*/
	Light(const Vector3& position, const Vector4& colour, Vector3 direction)
	{
		this->position = position;
		this->colour = colour;
		this->direction = direction;
	}

	~Light(void) {}

	Vector3 GetPosition() const					{ return position; }
	void	SetPosition(const Vector3& val)		{ position = val; }

	Vector4 GetColour() const					{ return colour; }
	void	SetColour(const Vector4& val)		{ colour = val; }

	float	GetRadius() const					{ return radius; }
	void	SetRadius(float val)				{ radius = val; }

	Vector3 GetDirection() const				{ return direction; }
	void	SetDirection(const Vector3& val)	{ direction = val; }

	void	Update(float dt) { position = Matrix4::Rotation(30.0f * dt, Vector3(0, 1, 0)) * position; };
	void	UpdateDirLight(float dt, float speed, Vector3 axis) 
	{
		this->direction		= Matrix4::Rotation(speed * dt, axis) * GetDirection();
		this->position		= Matrix4::Rotation(speed * dt, axis) * GetPosition();
		
		if (-direction.y >= 0)
		{
			this->colour.operator*(0.0f);
		}
		else
		{
			this->colour.operator*(-direction.y * 0.33f);
		}
	}

protected:
	Vector3 position;	Vector4 colour;
	
	float radius;
	Vector3 direction;
};