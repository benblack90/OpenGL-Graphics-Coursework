#pragma once
#include "Vector4.h"
#include "Vector3.h"
#include <string>

/*Abstract class for different light types: abstract, because in order to be fully general, the light has neither a position, nor a direction*/
class Light
{
public:
	Light() {};
	virtual ~Light() = 0 {};
	Vector4 GetColour() const { return colour; }
	void SetColour(const Vector4& val) { colour = val; }
	std::string GetName() const { return name; }

protected:
	Vector4 colour;
	Vector4 intensity;
	std::string name;
};

/*Direction light. All rays come from a single direction. Constructor ensures this value is a normal, for easy calculations*/
class DirectionLight : public Light
{
public:
	DirectionLight(Vector3 direction, const Vector4& colour)
	{
		direction.Normalise();
		this->direction = direction;
		this->colour = colour;
		name = "direction";
	}

	~DirectionLight(void) {};
	Vector3 GetDirection() const { return direction; }
	void SetDirection(const Vector3& val) { direction = val; }

protected:
	Vector3 direction;
};



class PointLight : public Light
{
public:
	PointLight(const Vector3& position, const Vector4& colour, float radius)
	{
		this->position = position;
		this->colour = colour;
		this->radius = radius;
		name = "point";
	}

	~PointLight(void) {};
	Vector3 GetPosition() const { return position; }
	void SetPosition(const Vector3& val) { position = val; }
	float GetRadius() const { return radius; }
	void SetRadius(float val) { radius = val; }
protected:
	Vector3 position;
	float radius;
};



/*Subclass of pointlight which specifies an angle of possible incidences. This is also expressed as a precalculated minimum dot product, for
easy comparison with other normals*/
class Spotlight : public PointLight
{
public:
	Spotlight(const Vector3& position, const Vector4& colour, float radius, float angle)
		:PointLight(position, colour, radius)
	{
		this->angle = angle;
		//using the cosine rule, A.B = |A| |B| cos(theta), we can 'convert' an angle to a dot product. Theta is the angle between a vector on the 
		//edge of the light, and a vector from the centre of the light. The angle parameter must thus be halved, to equal theta
		dotProdMin = cosf((angle / 2) * (PI/180));
		//2 degrees of extra softer light
		dimProdMin = cosf(((angle - 2) / 2) * (PI / 180));
		direction = Vector3(0, 0, -1);
		name = "spot";
	}
	~Spotlight(void) {};
	float GetAngle() const { return angle; }
	void SetAngle(float angle) { this->angle = angle; dotProdMin = cosf(angle / 2); }
	Vector3 GetDirection() const { return direction; }
	void SetDirection(const Vector3& dir) { direction = dir; }
	float GetDotProdMin() const { return dotProdMin; }
	float GetDimProdMin() const { return dimProdMin; }

protected:
	float angle;
	float dotProdMin;
	float dimProdMin;
	Vector3 direction;
};