#ifndef __RAYCAST_HIT__
#define __RAYCAST_HIT__

#include "MathGeoLib\include\MathGeoLib.h"

class GameObject;

class RaycastHit
{
public:
	RaycastHit();
	~RaycastHit();

	RaycastHit(float distance, math::float3 normal, math::float3 point, GameObject* object);

public:

	float distance = 0; //Distance from the ray's origin to the impact point.
	math::float3 normal = math::float3::zero; //Normal of the surface of the ray hit
	math::float3 point = math::float3::zero; //Impact point in world space
	GameObject* object = nullptr;
};

#endif // !__RAYCAST_HIT__
