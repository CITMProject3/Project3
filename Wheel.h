#ifndef __WHEEL_PHYSICS2__
#define __WHEEL_PHYSICS2__

#include "Globals.h"
#include "Application.h"

#include "MathGeoLib\include\MathGeoLib.h"

class ComponentCar;

class Wheel
{
public:
	Wheel(ComponentCar* _kart, float2 _posOffset, float3 _dir) : posOffset(_posOffset), dir(_dir), kart(_kart) {}

	void Cast();

	ComponentCar* kart;
private:
	float2 posOffset;
	float3 dir;

public:
	bool hit = false;
	float angleFromY = 0.0f;
	float distance = 0.0f;
	float3 hitNormal;
	float3 hitPoint;

};

#endif // !__WHEEL_PHYSICS2__
