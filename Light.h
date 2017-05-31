
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

struct LightInfo
{
	math::float3 ambient_color;
	float ambient_intensity;

	bool has_directional;
	math::float3 directional_color;
	float directional_intensity;
	math::float3 directional_direction;
	math::float4x4 directional_mat;
};