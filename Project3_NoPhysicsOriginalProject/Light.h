
#pragma once
#include "Color.h"
#include "MathGeoLib\include\MathGeoLib.h"

using namespace math;

struct Light
{
	Light();

	void Init();
	void SetPos(float x, float y, float z);
	void Active(bool active);
	void Render();

	Color ambient;
	Color diffuse;
	vec position;

	int ref;
	bool on;
};