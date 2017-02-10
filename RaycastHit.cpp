#include "RaycastHit.h"
#include "GameObject.h"

RaycastHit::RaycastHit()
{
}

RaycastHit::RaycastHit(float distance, math::float3 normal, math::float3 point, GameObject * object) : distance(distance), normal(normal), point(point), object(object)
{}

RaycastHit::~RaycastHit()
{
	object = nullptr;
}


