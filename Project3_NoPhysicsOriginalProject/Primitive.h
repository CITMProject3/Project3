
#pragma once
#include "MathGeoLib\include\MathGeoLib.h"
#include "Color.h"

using namespace math;

enum PrimitiveTypes
{
	Primitive_Point,
	Primitive_Line,
	Primitive_Plane,
	Primitive_Cube,
	Primitive_Sphere,
	Primitive_Cylinder
};

class Primitive
{
public:

	Primitive();

	virtual void	Render() const;
	virtual void	InnerRender() const;
	void			SetPos(float x, float y, float z);
	void			SetRotation(float angle, const vec &u);
	void			Scale(float x, float y, float z);
	PrimitiveTypes	GetType() const;
	vec				GetPos()const;
public:
	
	Color color;
	float4x4 transform;
	bool axis,wire;

protected:
	PrimitiveTypes type;
};

// ============================================
class Cube_P : public Primitive
{
public :
	Cube_P();
	Cube_P(float sizeX, float sizeY, float sizeZ);
	void InnerRender() const;
public:
	vec size;
};

// ============================================
class Sphere_P : public Primitive
{
public:
	Sphere_P();
	Sphere_P(float radius);
	void InnerRender() const;
public:
	float radius;
};

// ============================================
class Cylinder_P : public Primitive
{
public:
	Cylinder_P();
	Cylinder_P(float radius, float height);
	void InnerRender() const;
public:
	float radius;
	float height;
};

// ============================================
class Line_P : public Primitive
{
public:
	Line_P();
	Line_P(float x, float y, float z);
	void InnerRender() const;
public:
	vec origin;
	vec destination;
};

// ============================================
class Plane_P : public Primitive
{
public:
	Plane_P();
	Plane_P(float x, float y, float z, float d);
	void InnerRender() const;
public:
	vec normal;
	float constant;
};